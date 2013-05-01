#include "kmeans.h"
#include <vector>
#include <map>
#include <cassert>
#include <iostream>

using namespace std;

DNAStrand::DNAStrand(string s) {
	strand = s;
}

float DNAStrand::getDistance(DNAStrand s) {
	int i;
	float diff = 0;

	assert(strand.size() == s.strand.size());
	for (i = 0; i < strand.size(); i++) {
		if (strand[i] != s.strand[i]) {
			diff += 1;
		}
	}
	return diff;
}

void DNAStrand::printSelf() {
	cout << strand;
}

// FIXME: This may have problem, may have more than one mean
// We need to consider them all
DNAStrand DNAStrand::getMean(vector<DNAStrand> strands) {
	int i, j;
	int length;
	char c;
	float num;
	DNAStrand newstrand;
	vector<map<char, float> > count;

	assert(strands.size() > 0);

	// initialize count
	length = strands[0].strand.size();
    count.resize(length);
    for (i = 0; i < length; i++) {
    	count[i]['A'] = 0;
    	count[i]['C'] = 0;
    	count[i]['G'] = 0;
    	count[i]['T'] = 0;
    }

    // count occurance of each base in each position
 	for (i = 0; i < strands.size(); i++) {
		for (j = 0; j < length; j++) {
			switch(strands[i].strand[j]) {
				case 'A':
					count[j]['A'] += 1;
					break;
				case 'C':
					count[j]['C'] += 1;
					break;
				case 'G':
					count[j]['G'] += 1;
					break;
				case 'T':
					count[j]['T'] += 1;
					break;
				default:
					// die
					assert(0);
			}
		}
	}

	for (i = 0; i < length; i++) {
		c = 'A';
		num = count[i]['A'];
		if (count[i]['C'] > num) {
			c = 'C';
		} else if (count[i]['G'] > num) {
			c = 'G';
		} else if (count[i]['T'] > num) {
			c = 'T';
		}
		newstrand.strand[i] = c;
	}
}
