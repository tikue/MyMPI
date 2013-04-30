#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    // MPI vars
    int numprocs, rank, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);
    MPI_Finalize();

    // getopt vars
    char opt;
    int k = 5;
    char *filename = "data.txt";

    while ((opt = getopt(argc, argv, "k:f:")) != EOF) {
        switch(opt) {
        case 'k':
            k = atoi(optarg); 
            break;
        case 'f':
            filename = optarg;
            break;
        default:
            printf("Usage: %s -k <k> -f <filename>\n", argv[0]);
            return 0;
        }
    }
    exit(0);
}
