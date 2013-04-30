#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <mpi.h>
#include "kmeans.h"

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
    int k = 5, linelen = 32, numlines = 100, range = 100;
    char *filename = "data.txt";

    while ((opt = getopt(argc, argv, "k:r:l:n:f:")) != EOF) {
        switch(opt) {
        case 'k':
            k = atoi(optarg); 
            break;
        case 'r':
            range = atoi(optarg);
        case 'l':
            linelen = atoi(optarg);
        case 'n':
            numlines = atoi(optarg);
        case 'f':
            filename = optarg;
            break;
        default:
            printf("Usage: %s -k <k> -f <filename>\n", argv[0]);
            return 0;
        }
    }
    fileinfo info = {
        .range = range,
        .linelen = linelen,
        .numlines = numlines,
        .filename = filename
    };
    kmeans(rank, numprocs, k, info);
    MPI_Finalize();
    exit(0);
}
