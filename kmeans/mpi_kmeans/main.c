#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <mpi.h>
#include "mpi_kmeans.h"

void usage(char *name) {
    printf("Usage: %s [-k <k>] [-l <linelen>] "
            "[-n <numlines>] [-f <filename>]\n", name);
}

int main(int argc, char *argv[]) {
    // MPI vars
    int numprocs, rank, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);

    // getopt vars
    char opt;
    int k = 9, linelen = 32, numlines = 300;
    char *filename = "data.txt";

    while ((opt = getopt(argc, argv, "k:l:n:f:h")) != EOF) {
        switch(opt) {
        case 'k':
            k = atoi(optarg); 
            break;
        case 'l':
            linelen = atoi(optarg);
        case 'n':
            numlines = atoi(optarg);
        case 'f':
            filename = optarg;
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

    kmeans(rank, numprocs, k, info);
    MPI_Finalize();
    exit(0);
}
