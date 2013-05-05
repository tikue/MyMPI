#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include <mpi.h>
#include <assert.h>
#include "mpi_kmeans.h"

float distance(char* a, char* b);
void normalize(int *arr, int numprocs);
void scatterdnadata(char *all, int total, char *dnas,
	int revcnt, int np, int rank);
int initdna(char *dnas, fileinfo info);
void senddnameans(int k, char *means, int rank);
void initdnameans(int k, char* means, int total, char *all);

int dna_kmeans(int rank, int numprocs, int k, fileinfo info) {
    // read in dna strands and initialize means
    dnalen = info.linelen + 1; // \0 teminated
    char* alldna = malloc(sizeof(char) * info.numlines * dnalen);
    char* means = malloc(sizeof(char) * k * dnalen);

    memset(alldna, 0, sizeof(alldna));
    memset(means, 0, sizeof(means));

    if (!rank) {
	initdna(alldna, info);
        initdnameans(k, means, info.numlines, alldna);
    }
    senddnameans(k, means, rank);
    

    // scatter the data
    int remainder = info.numlines % numprocs;
    int recvcnt = info.numlines / numprocs + (rank < remainder ? 1:0);
    char* dnas = malloc(sizeof(char) * recvcnt*dnalen);
    scatterdnadata(alldna, info.numlines, dnas, recvcnt, numprocs, rank);

    // iterate
    int changed;
    do {
        changed = 0;
        char *kmeans[k][recvcnt];
        int counts[k];
        memset(kmeans, 0, sizeof(kmeans));
        memset(counts, 0, sizeof(counts));

        // calculate recvcnt distances
        for (int i = 0; i < recvcnt; i++) {
            int closest = 0;
            float dist = distance(means, &dnas[i*dnalen]);
            float jdist;
            for (int j = 1; j < k; j++) {
                jdist = distance(&means[j*dnalen], &dnas[i*dnalen]);
                if (jdist < dist) {
                    closest = j;
                    dist = jdist;
		}
            }
            kmeans[closest][counts[closest]++] = dnas + i*dnalen;
        }
        
        // reduce clusters to centroids
        for (int i = 0; i < k; i++) {
	    char newmean[dnalen];
	    memset(newmean, 0, sizeof(newmean));
	    // for each position
	    for (int t = 0; t < dnalen - 1; t++) {
            int freq[4];
            memset(freq, 0, sizeof(freq));
            for (int j = 0; j < counts[i]; j++) {
                switch(kmeans[i][j][t]) {
                case 'A':
                    freq[0]++;
                    break;
                case 'C':
                    freq[1]++;
                    break;
                case 'G':
                    freq[2]++;
                    break;
                case 'T':
                    freq[3]++;
                    break;
                default:
                    assert(0);
                }
            }

            int countA, countC, countG, countT;
            MPI_Reduce(freq, &countA, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&freq[1], &countC, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&freq[2], &countG, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&freq[3], &countT, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

            if (!rank) {
                newmean[t] = 'A';
                int cnt = countA;
                if (countC > cnt) {
                    newmean[t] = 'C';
                cnt = countC;
                }
                if (countG > cnt) {
                    newmean[t] = 'G';
                cnt = countG;
                }
                if (countT > cnt) {
                    newmean[t] = 'T';
                    cnt = countT;
                }
            }
	    }

            if (!rank) {
	        if (strncmp(means+i*dnalen, newmean, dnalen-1)) {
                    changed = 1;
/*
                    printf("updating mean[%d]: %s-->%s\n",
                        i, means+i*dnalen, newmean);
*/
		    memcpy(means+i*dnalen, newmean, dnalen-1);
                }

            }
        }
        MPI_Bcast(&changed, 1, MPI_INT, 0, MPI_COMM_WORLD);
        senddnameans(k, means, rank);
        if (!rank) {
/*
            for (int i = 0; i < k; i++)
                printf("%d: %s\n", i, means+i*dnalen);
            printf("\n");
*/
        }
    } while (changed);
    if (!rank)
        printf("done.\n");
    free(alldna);
    free(means);
    free(dnas);
}

void initdnameans(int k, char* means, int total, char *all) {
    srand(time(NULL));
    int meanis[k];
    memset(meanis, -1, k);

    for (int i = 0; i < k; i++) {
        int index = rand() % total;
        if (k < total) {
	    // this is just to make sure we get different
	    // dna every time
            while (in(k, meanis, index))
                index = rand() % total;
	}
            
        meanis[i] = index;
	memcpy(means+i*dnalen, all+index*dnalen, (dnalen)*sizeof(char));
<<<<<<< HEAD
    }
=======
//	printf("%s\n", means+i*dnalen);
	
    }
/*
    printf("initmeans:\n");
    for (int i = 0; i < k; i++) {
        printf("%s\n", means+i*dnalen);
    }
    printf("\n");
*/
>>>>>>> 5b54fea8de9038c89b5fd5f62aa5f983a7095610
}

void senddnameans(int k, char *means, int rank) {
    MPI_Bcast(means, k*dnalen, MPI_CHAR, 0, MPI_COMM_WORLD);
}

int initdna(char *dnas, fileinfo info) {
    FILE *fp;
    if (!(fp = fopen(info.filename, "r")))
        return -1;

    char line[info.linelen+1];
    int num = 0;
    while (fgets(line, sizeof(line), fp)!= NULL) {
       if (line[strlen(line)] == '\n')
           line[strlen(line)] = '\0';
       if (strlen(line) < info.linelen) continue;
       strncpy(dnas + num, line, strlen(line));
       num += strlen(line);
       dnas[num++] = '\0';
    }
    fclose(fp);
    return 0;
}

void scatterdnadata(char *all, int total, char *dnas,
	 int revcnt, int np, int rank) {
    int sendcnts[np], displs[np];
    getsendcnts(sendcnts, total, np);
    getdispls(displs, sendcnts, np);

    // normalize the size
    normalize(sendcnts, np);
    normalize(displs, np);
    revcnt *= dnalen;

    MPI_Scatterv(all, sendcnts, displs, MPI_CHAR, dnas, revcnt,
                MPI_CHAR, 0, MPI_COMM_WORLD);
}

void normalize(int *arr, int numprocs) {
    for (int i=0; i < numprocs; i++) {
        arr[i] *= dnalen;
    }
}

float distance(char* a, char* b) {
    float diff = 0;
    for (int i = 0; i < dnalen; i++) {
        if (a[i] != b[i]) {
            diff++;
        }
    }
    return diff;
}
