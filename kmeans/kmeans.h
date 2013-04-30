#ifndef __KMEANS_H__
#define __KMEANS_H__

typedef struct point_s {
    float x, y;
} point;

typedef struct fileinfo_s {
    const int range;
    const int linelen;
    const int numlines;
    const char *filename;
} fileinfo;

int kmeans(int rank, int numprocs, int k, fileinfo info);
void initmeans(int k, point *means, int rank, int range);
void sendmeans(int k, point *means, int rank, int range);
int initpoints(point *points, fileinfo info);
void getsendcnts(int *sendcnts, int numlines, int numprocs);
void getdispls(int *displs, int *sendcnts, int numprocs);
float eucliddist(point a, point b);

#endif
