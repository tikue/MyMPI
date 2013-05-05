#ifndef _SEQ_KMEANS_HPP
#define _SEQ_KMEANS_HPP

#include "kmeans.h"
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

template<class DataType>
class SeqKmeans {
public:
        vector<int> label; // label which cluster each data belongs to
        vector<DataType> centroids; // the centroids for each cluster

        SeqKmeans(string, int, float);

        void kmeans();
        void readData(string file_name);
        void save();

private:
        int N; // N data
        int K; // K cluster
        float tolerance; // decide when to converge
        vector<DataType> data;
        void allocateMemory();
        void initialize(); // initialize K centroids    
};


template <class DataType> SeqKmeans<DataType>::SeqKmeans(string file_name,
    int K, float tolerance) : K(K), tolerance(tolerance) {
    readData(file_name);
    assert(N > 0 && K > 0 && K <= N && tolerance >= 0);
    allocateMemory();
}

template <class DataType> void SeqKmeans<DataType>::allocateMemory() {
    label.resize(N);
    centroids.resize(K);
}

template <class DataType> void SeqKmeans<DataType>::readData(string file_name) {
    string line;
    ifstream infile(file_name.c_str());
    DataType d;
    while (getline(infile, line)) {
        d = DataType(line);
        data.push_back(d);
    }
    N = data.size();
}

template <class DataType> void SeqKmeans<DataType>::initialize() {
    /* pick every N/K th data objects */
    int i;
    //cout << "Initial data: " << endl;
    for (i = 0; i < K; i++) {
        centroids[i] = data[i * N/K];
	//centroids[i].printSelf();
	//cout << endl;
    }
    //cout << endl;
}

template <class DataType> void SeqKmeans<DataType>::kmeans() {
    vector<vector<DataType> > clusters(K);
    int i, j;
    int index;
    float delta = 0, min_dist, dist;
    int epoc = 0;

    initialize();

    do {
	//cout << "epoc: " << epoc << endl;
	epoc++;
	delta = 0;

	for (i = 0; i < K; i++) {
	    clusters[i].clear();
	}

        // assign each data to a cluster
        for (i = 0; i<N; i++) {
            index = 0;
            min_dist = data[i].getDistance(centroids[0]);
            for (j = 1; j < K; j++) {
                dist = data[i].getDistance(centroids[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    index = j;
                }
            }
           
            // if label changes, increase delta by 1
            if (label[i] != index) delta += 1.0;

            label[i] = index;
        }

        // update the centroids
        for (i = 0; i < N; i++) {
            clusters[label[i]].push_back(data[i]);
        }

        for (j = 0; j < K; j++) {
	    //cout << "cluster " << j << ": " << endl;
	    for (i = 0; i < clusters[j].size(); i++) {
		//cout << i << " :";
		//clusters[j][i].printSelf();
		//cout << endl;
	    }
            centroids[j] = DataType::getMean(clusters[j]);
	    //cout << "centroid: ";
	    //centroids[j].printSelf();
	    //cout << endl;
        }
	//cout << endl;
    } while (delta > tolerance);;
}


template <class DataType> void SeqKmeans<DataType>::save() {
    int i;
    //cout << "K centroids:" << endl;
    for (i = 0; i < K; i++) {
        //cout << "i: ";
	//centroids[i].printSelf();
	//cout << endl;
    }
    //cout << endl;
    //cout << "Cluster assignment:" << endl;
    for (i = 0; i < N; i++) {
        //cout << "i: ";
 	//data[i].printSelf();
	//cout << "   - cluster " << label[i] << endl;
    }
}

#endif
