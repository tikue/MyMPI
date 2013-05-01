#ifndef _KEMANS_HPP
#define _KEMANS_HPP

#include <vector>
#include <string>
#include <cassert>

using namespace std;

class Point {
public:
	float x, y;
	Point() {};
	Point(string);
	Point(float, float);
	float getDistance(Point);
	void printSelf();
	static Point getMean(vector<Point>);
};

class DNAStrand {
public:
	string strand;
	DNAStrand() {}
	DNAStrand(string);
	float getDistance(DNAStrand);
	void printSelf();
	static DNAStrand getMean(vector<DNAStrand>);
};

#endif
