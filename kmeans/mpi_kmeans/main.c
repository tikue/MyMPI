#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>
#include "mpi_kmeans.h"

void usage(char *name) {
    printf("Usage: %s [-k <k>] [-l <linelen>] "
            "[-n <numlines>] [-f <filename>] [-p|-d]\n", name);
}

int main(int argc, char *argv[]) {
    // MPI vars
    int numprocs, rank, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int choice = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);

    // getopt vars
    char opt;
    int k = 20, linelen = 32, numlines = 400;
    char *filename = "input/points.txt";

    while ((opt = getopt(argc, argv, "k:l:n:f:pdh")) != EOF) {
        switch(opt) {
        case 'k':
            k = atoi(optarg); 
            break;
        case 'l':
            linelen = atoi(optarg);
            break;
        case 'n':
            numlines = atoi(optarg);
            break;
        case 'f':
            filename = optarg;
            break;
	case 'p':
	    choice = 0;
	    break;
	case 'd':
	    choice = 1;
	    break;
        case 'h':
        default:
            usage(argv[0]);
            MPI_Finalize();
            return 0;
        }
    }
    assert(numlines > k);
    fileinfo info = {
        .linelen = linelen,
        .numlines = numlines,
        .filename = filename
    };

    int runs = 5;
    float mean_time = 0;
    float clocks_per_sec = 1.0 * CLOCKS_PER_SEC;
    for (int i = 0; i < runs; i++) {
        time_t start = clock();
        if (!choice)
            kmeans(rank, numprocs, k, info);
	else
            dna_kmeans(rank, numprocs, k, info);
        time_t end = clock();
        float runtime = (end - start) / clocks_per_sec;
        mean_time += runtime;
        if (!rank)
            printf("wall time: %f\n", runtime);
    }
    mean_time /= runs;
    if (!rank)
        printf("average wall time: %f\n", mean_time);
    MPI_Finalize();
    exit(0);
}
