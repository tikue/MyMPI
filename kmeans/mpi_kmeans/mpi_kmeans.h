#ifndef __KMEANS_H__
#define __KMEANS_H__

/*
typedef struct point_s {
    float x, y;
} point;
*/
typedef float point[2];

typedef struct fileinfo_s {
    const int linelen;
    const int numlines;
    const char *filename;
} fileinfo;

int in(int n, int *a, int t);
int kmeans(int rank, int numprocs, int k, fileinfo info);
int update_mean(point mean, point *partial, int partial_cnt, int rank);
void initmeans(int k, point *means, int total, point *all);
void sendmeans(int k, point *means, int rank);
int initpoints(point *points, fileinfo info);
void getsendcnts(int *sendcnts, int numlines, int numprocs);
void getdispls(int *displs, int *sendcnts, int numprocs);
void scatterdata(point *all, int sum, point *pts, int cnt, int np, int rank);
float eucliddist(point a, point b);

#endif
