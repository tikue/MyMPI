#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <mpi.h>
#include "kmeans.h"

int kmeans(int rank, int numprocs, int k, fileinfo info) {

    // initialize means to random values
    point means[k];
    initmeans(k, means, rank, info.range);
    sendmeans(k, means, rank, info.range);

    // read in data points
    point allpoints[info.numlines];
    float allxs[info.numlines], allys[info.numlines];
    if (!rank) {
        initpoints(allpoints, info);
        point p;
        for (int i = 0; i < info.numlines; i++) {
            p = allpoints[i];
            allxs[i] = p.x;
            allys[i] = p.y;
        }
    }

    // scatter the data
    int sendcnts[numprocs], displs[numprocs];
    getsendcnts(sendcnts, info.numlines, numprocs);
    getdispls(displs, sendcnts, numprocs);
    int recvcnt = sendcnts[rank];
    point points[recvcnt];
    float xs[recvcnt], ys[recvcnt];
    MPI_Scatterv(allxs, sendcnts, displs, MPI_FLOAT, xs, recvcnt,
                MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(allys, sendcnts, displs, MPI_FLOAT, ys, recvcnt,
                MPI_FLOAT, 0, MPI_COMM_WORLD);

    // construct the point arrays
    for (int i = 0; i < recvcnt; i++)
        points[i] = (point) {
            .x = xs[i],
            .y = ys[i]
        };

    // iterate
    point rootkmeans[k][info.numlines];
    point kmeans[k][recvcnt];
    int counts[k];
    int changed;

    do {
        changed = 0;
        memset(rootkmeans, 0, sizeof(rootkmeans));
        memset(kmeans, 0, sizeof(kmeans));
        memset(counts, 0, sizeof(counts));

        // calculate $recvcnt distances
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
        
        for (int i = 0; i < k; i++) {
            point *tosend = kmeans[i];
            int count = counts[i];
            int total[numprocs];
            for (int j = 0; j < count; j++) {
                point sendpt = tosend[j];
                xs[j] = sendpt.x;
                ys[j] = sendpt.y;
            }
            MPI_Reduce(&count, total, 1, MPI_INT, MPI_SUM, 0,
                    MPI_COMM_WORLD);
            MPI_Reduce(xs, allxs, count, MPI_FLOAT, MPI_SUM, 0,
                    MPI_COMM_WORLD);
            MPI_Reduce(ys, allys, count, MPI_FLOAT, MPI_SUM, 0,
                    MPI_COMM_WORLD);
            
            if (!rank) {
                int cluster = total[0];
                if (cluster == 0)
                    continue;
                float x = allxs[0] / cluster;
                float y = allys[0] / cluster;
                point mean = means[i];
                if (x != mean.x || y != mean.y) {
                    changed = 1;
                    means[i] = (point) {
                        .x = x,
                        .y = y
                    };
                }
            }
        }
        MPI_Bcast(&changed, 1, MPI_INT, 0, MPI_COMM_WORLD);
        sendmeans(k, means, rank, info.range);
        for (int i = 0; i < k; i++)
            printf("(%f, %f) ", means[i].x, means[i].y);
        printf("\n");
    } while (changed);
}

void initmeans(int k, point *means, int rank, int range) {
    if (!rank) {
        for (int i = 0; i < k; i++)
            means[i] = (point) {
                .x = rand() % range,
                .y = rand() % range
            };
    }
}

void sendmeans(int k, point *means, int rank, int range) {
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
