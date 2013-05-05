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
            point p;
            memcpy(p, points[i], sizeof(p));
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
            memcpy(kmeans[closest][counts[closest]++], p, sizeof(p));
        }
        
        // reduce clusters to centroids
        for (int i = 0; i < k; i++) {
            // calculate mean
            if (update_mean(means[i], kmeans[i], counts[i], rank))
                changed = 1;
        }
        MPI_Bcast(&changed, 1, MPI_INT, 0, MPI_COMM_WORLD);
        sendmeans(k, means, rank);
        if (!rank) {
            for (int i = 0; i < k; i++)
                printf("%d:(%f, %f)\n", i, means[i][0], means[i][1]);
            printf("\n");
        }
    } while (changed);
    if (!rank)
        printf("done.\n");
}

int update_mean(point mean, point *partial, int partial_cnt, int rank) {
    int changed = 0;
    // get cluster size
    int cluster;
    MPI_Reduce(&partial_cnt, &cluster, 1, MPI_INT, MPI_SUM, 0,
            MPI_COMM_WORLD);

    point meani[partial_cnt];
    memcpy(meani, partial, sizeof(meani));

    point p = {0, 0};
    for (int j = 0; j < partial_cnt; j++) {
        point pt;
        memcpy(pt, meani[j], sizeof(pt));
        p[0] += pt[0];
        p[1] += pt[1];
    }
    point sum_p;
    MPI_Reduce(p, sum_p, 2, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (!rank) {
        float x = sum_p[0] / cluster;
        float y = sum_p[1] / cluster;
        if (x != mean[0] || y != mean[1]) {
            changed = 1;
            printf("updating mean: (%f, %f)-->(%f, %f)\n",
                    mean[0], mean[1], x, y);
            mean[0] = x;
            mean[1] = y;
        }
    }
    return changed;
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
            
        meanis[i] = index;
        memcpy(means[i], all[index], sizeof(means[i]));
    }
    printf("initmeans:\n");
    for (int i = 0; i < k; i++) {
        printf("(%f, %f)\n", means[i][0], means[i][1]);
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
    MPI_Bcast(means, 2 * k, MPI_FLOAT, 0, MPI_COMM_WORLD);
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
        point p = {
            atof(line),
            atof(strchr(line, ',') + 1)
        };
        memcpy(points[numpoints++], p, sizeof(p));
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
    MPI_Scatterv(all, sendcnts, displs, MPI_FLOAT, pts, 2*cnt, MPI_FLOAT,
            0, MPI_COMM_WORLD);
}
    
// number of floats to send per proc; i.e. 2*(points to send)
void getsendcnts(int *sendcnts, int numlines, int numprocs) {
    int sendcnt = 2 * numlines / numprocs;
    int leftovers = numlines % numprocs;
    for (int i = 0; i < leftovers; i++)
        sendcnts[i] = sendcnt + 2;
    for (int i = leftovers; i < numprocs; i++)
        sendcnts[i] = sendcnt;
}

void getdispls(int *displs, int *sendcnts, int numprocs) {
    displs[0] = 0;
    for (int i = 1; i < numprocs; i++)
        displs[i] = displs[i-1] + sendcnts[i-1];
}

float eucliddist(point a, point b) {
    return sqrt(pow(a[0] - b[0], 2) + pow(a[1] - b[1], 2));
}
