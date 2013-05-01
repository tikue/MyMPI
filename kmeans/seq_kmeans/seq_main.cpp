#include "kmeans.h"
#include "seq_kmeans.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

using namespace std;

void usage() {
	cout << "seqkmeans -<p|d> <input_file> <K_clusters> <tolerance>" << endl;
	cout << "	-p: for data points" << endl;
	cout << "	-d: for DNA strands" << endl;
	cout << "	tolerance: the algorithm will stop after no more than" << endl; 
	cout << " 		   tolerance number of data points change their" << endl;
	cout << "		   cluster assignments" << endl;
}

int main(int argc, char *argv[]) {
	string file_name;
	int choice;

	if (argc < 3) {
		usage();
		exit(-1);
	}

	if (!strncmp("-p", argv[1], 2)) {
		choice = 0;
	} else if (!strncmp("-d", argv[1], 2)) {
		choice = 1;
	} else {
		usage();
		exit(-1);
	}

	file_name = argv[2];


	if (choice == 0) {
		SeqKmeans <Point> kmeans(file_name, atoi(argv[3]), atof(argv[4]));
		kmeans.kmeans();
		kmeans.save();
	} else {
		SeqKmeans<DNAStrand> kmeans(file_name, atoi(argv[3]), atof(argv[4]));
		kmeans.kmeans();
		kmeans.save();
	}
}
