#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include <mpi.h>
#include "mpi_kmeans.h"

int kmeans(int rank, int numprocs, int k, fileinfo info) {

    // read in data points and initialize means
    point allpoints[info.numlines];
    point means[k];
    if (!rank) {
        initpoints(allpoints, info);
        initmeans(k, means, info.numlines, allpoints);
    }
    sendmeans(k, means, rank);

    // scatter the data
    int remainder = info.numlines % numprocs;
    int recvcnt = info.numlines / numprocs + (rank < remainder ? 1:0);
    point points[recvcnt];
    scatterdata(allpoints, info.numlines, points, recvcnt, numprocs, rank);

    // iterate
    int changed;
    do {
        changed = 0;
        point rootkmeans[k][info.numlines];
        point kmeans[k][recvcnt];
        int counts[k];
        memset(rootkmeans, 0, sizeof(rootkmeans));
        memset(kmeans, 0, sizeof(kmeans));
        memset(counts, 0, sizeof(counts));

        // calculate recvcnt distances
        for (int i = 0; i < recvcnt; i++) {
            point p = points[i];
            int closest = 0;
            float dist = eucliddist(means[0], p);
            float jdist;
            for (int j = 1; j < k; j++) {
                jdist = eucliddist(means[j], p);
                if (jdist < dist) {
                    closest = j;
                    dist = jdist;
                }
            }
            kmeans[closest][counts[closest]++] = p;
        }
        
        // reduce clusters to centroids
        for (int i = 0; i < k; i++) {
            point *meani = kmeans[i];
            int count = counts[i];
            int cluster;
            MPI_Allreduce(&count, &cluster, 1, MPI_INT, MPI_SUM,
                    MPI_COMM_WORLD);

            float x = 0, y = 0;
            for (int j = 0; j < count; j++) {
                point pt = meani[j];
                x += pt.x;
                y += pt.y;
            }
            float sum_x;
            MPI_Reduce(&x, &sum_x, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

            float sum_y;
            MPI_Reduce(&y, &sum_y, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
            
            if (!rank) {
                float x = sum_x / cluster;
                float y = sum_y / cluster;
                point mean = means[i];
                if (x != mean.x || y != mean.y) {
                    changed = 1;
                    printf("updating mean[%d]: (%f, %f)-->(%f, %f)\n",
                        i, mean.x, mean.y, x, y);
                    means[i] = (point) {
                        .x = x,
                        .y = y
                    };
                }
            }
        }
        MPI_Bcast(&changed, 1, MPI_INT, 0, MPI_COMM_WORLD);
        sendmeans(k, means, rank);
        if (!rank) {
            for (int i = 0; i < k; i++)
                printf("%d:(%f, %f)\n", i, means[i].x, means[i].y);
            printf("\n");
        }
    } while (changed);
    if (!rank)
        printf("done.\n");
}

void initmeans(int k, point *means, int total, point *all) {
    srand(time(NULL));
    int meanis[k];
    memset(meanis, -1, k);

    for (int i = 0; i < k; i++) {
        int index = rand() % total;
        if (k < total)
            while (in(k, meanis, index))
                index = rand() % total;
            
        point p = all[index];
        meanis[i] = index;
        means[i] = (point) {
            .x = p.x,
            .y = p.y
        };
    }
    printf("initmeans:\n");
    for (int i = 0; i < k; i++) {
        point mean = means[i];
        printf("(%f, %f)\n", mean.x, mean.y);
    }
    printf("\n");
}

int in(int n, int *a, int t) {
    for (int i = 0; i < n; i++)
        if (a[i] == t)
            return 1;
    return 0;
}

void sendmeans(int k, point *means, int rank) {
    float xs[k], ys[k];
    point mean;
    if (!rank)
        for (int i = 0; i < k; i++) {
            mean = means[i];
            xs[i] = mean.x;
            ys[i] = mean.y;
        }
    MPI_Bcast(xs, k, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(ys, k, MPI_FLOAT, 0, MPI_COMM_WORLD);
    if (rank)
        for (int i = 0; i < k; i++)
            means[i] = (point) {
                .x = xs[i],
                .y = ys[i]
            };
}

int initpoints(point *points, fileinfo info) {
    FILE *fp;
    if (!(fp = fopen(info.filename, "r")))
        return -1;

    char *line = NULL;
    size_t len = 0; 
    size_t read;
    int numpoints = 0;
    for (int lineno = 0; lineno < info.numlines; lineno++) {
        read = getline(&line, &len, fp);
        float x = atof(line);
        float y = atof(strchr(line, ',') + 1);
        points[numpoints++] = (point) {
            .x = x,
            .y = y
        };
    }
    if (line)
        free(line);
    fclose(fp);
    return 0;
}

void scatterdata(point *all, int sum, point *pts, int cnt, int np, int rank) {
    int sendcnts[np], displs[np];
    getsendcnts(sendcnts, sum, np);
    getdispls(displs, sendcnts, np);
    float allxs[sum], allys[sum];
    if (!rank) {
        point p;
        for (int i = 0; i < sum; i++) {
            p = all[i];
            allxs[i] = p.x;
            allys[i] = p.y;
        }
    }
    float xs[cnt], ys[cnt];
    MPI_Scatterv(allxs, sendcnts, displs, MPI_FLOAT, xs, cnt,
                MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(allys, sendcnts, displs, MPI_FLOAT, ys, cnt,
                MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    // construct the point arrays
    for (int i = 0; i < cnt; i++)
        pts[i] = (point) {
            .x = xs[i],
            .y = ys[i]
        };
}
    
void getsendcnts(int *sendcnts, int numlines, int numprocs) {
    int sendcnt = numlines / numprocs;
    int leftovers = numlines % numprocs;
    for (int i = 0; i < leftovers; i++)
        sendcnts[i] = sendcnt + 1;
    for (int i = leftovers; i < numprocs; i++)
        sendcnts[i] = sendcnt;
}

void getdispls(int *displs, int *sendcnts, int numprocs) {
    displs[0] = 0;
    for (int i = 1; i < numprocs; i++)
        displs[i] = displs[i-1] + sendcnts[i-1];
}

float eucliddist(point a, point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
