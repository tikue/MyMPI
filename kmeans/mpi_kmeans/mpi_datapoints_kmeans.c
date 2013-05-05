#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include <mpi.h>
#include "mpi_kmeans.h"

// mean function that initializes data and performs kmeans
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
    int iters = 0;
    do {
        changed = 0;
        point partialmeans[k];
        int counts[k];
        memset(partialmeans, 0, sizeof(partialmeans));
        memset(counts, 0, sizeof(counts));

        // determine assignments and sum partial means
        for (int i = 0; i < recvcnt; i++) {
            int m = asgncluster(k, means, points[i]);
            addpt(partialmeans[m], points[i]);
            counts[m]++;
        }
        
        // reduce clusters to centroids
        changed = update_means(k, means, partialmeans, counts, rank);

        MPI_Bcast(&changed, 1, MPI_INT, 0, MPI_COMM_WORLD);
        sendmeans(k, means, rank);
        if (!rank) printmeans(k, means, 0);
    } while (changed && ++iters < 100);
    if (!rank) printmeans(k, means, 1);
    if (!rank) printf("iters: %d, ", iters);

}

void addpt(point sum, point p) {
    sum[0] += p[0];
    sum[1] += p[1];
}

void printmeans(int k, point *means, int nonumbers) {
    printf("updated means:\n");
    for (int i = 0; i < k; i++) {
        if (!nonumbers)
            printf("%d: ", i);
        printf("(%f, %f)\n", means[i][0], means[i][1]);
    }
    printf("\n");
}

int asgncluster(int k, point *means, point p) {
    int closest = 0;
    float dist = eucliddist(means[0], p);
    float disti;
    for (int i = 1; i < k; i++) {
        disti = eucliddist(means[i], p);
        if (disti < dist) {
            closest = i;
            dist = disti;
        }
    }
    return closest;
}

// given a cluster of points on each proc, reduce to the mean
int update_means(int k, point *means, point *partials, int *counts, int rank) {
    int changed = 0;
    // get cluster size
    int clusters[k];
    point newmeans[k];

    MPI_Allreduce(counts, clusters, k, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Reduce(partials, newmeans, 2 * k, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    for (int i = 0; i < k; i++) {
        int cluster = clusters[i];
        if (!cluster)
            continue;

        if (!rank) {
            point mean;
            memcpy(mean, means[i], sizeof(mean));

            float x = newmeans[i][0] / cluster;
            float y = newmeans[i][1] / cluster;

            if (x != mean[0] || y != mean[1]) {
                changed = 1;
                mean[0] = x;
                mean[1] = y;
                memcpy(means[i], mean, sizeof(mean));
            }
        }
    }
    return changed;
}

// initialize random means by choosing points from data
void initmeans(int k, point *means, int total, point *all) {
    srand(0); // arbitrary but non-random seed for data analysis
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
}

// check if int n is in array a of size t
int in(int n, int *a, int t) {
    for (int i = 0; i < n; i++)
        if (a[i] == t)
            return 1;
    return 0;
}

// update 
void sendmeans(int k, point *means, int rank) {
    MPI_Bcast(means, 2 * k, MPI_FLOAT, 0, MPI_COMM_WORLD);
}

// read in data from a file
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

// each proc gets a fraction of the total data points
void scatterdata(point *all, int sum, point *pts, int cnt, int np, int rank) {
    int sendcnts[np], displs[np];
    getsendcnts(sendcnts, sum, np);
    for (int i = 0; i < np; i++)
        sendcnts[i] *= 2;
    getdispls(displs, sendcnts, np);
    MPI_Scatterv(all, sendcnts, displs, MPI_FLOAT, pts, 2*cnt, MPI_FLOAT,
            0, MPI_COMM_WORLD);
}
    
// number of floats to send per proc; i.e. 2*(points to send)
void getsendcnts(int *sendcnts, int numlines, int numprocs) {
    int sendcnt = numlines / numprocs;
    int leftovers = numlines % numprocs;
    for (int i = 0; i < leftovers; i++)
        sendcnts[i] = sendcnt + 1;
    for (int i = leftovers; i < numprocs; i++)
        sendcnts[i] = sendcnt;
}

// displacements used in scatterv
void getdispls(int *displs, int *sendcnts, int numprocs) {
    displs[0] = 0;
    for (int i = 1; i < numprocs; i++)
        displs[i] = displs[i-1] + sendcnts[i-1];
}

// euclidean distance between two points
float eucliddist(point a, point b) {
    return sqrt(pow(a[0] - b[0], 2) + pow(a[1] - b[1], 2));
}
