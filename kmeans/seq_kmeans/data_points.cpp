#include "kmeans.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cassert>

using namespace std;

Point::Point(string s) {
	x = atof(s.c_str());
	y = atof(strchr(s.c_str(), ',') + 1);
}

Point::Point(float a, float b) {
	x = a;
	y = b;
}

float Point::getDistance(Point another) {
	return pow(x - another.x, 2) + pow(y - another.y, 2);
}

void Point::printSelf() {
	cout << x << "	" << y;
}

Point Point::getMean(vector<Point> points) {
	int i;
	int N;
	Point point;
	
	N = points.size();
	assert(N > 0);

	point.x = 0;
	point.y = 0;
	for (i = 0; i < N ; i++) {
		point.x += points[i].x;
		point.y += points[i].y;
	}
	point.x /= N;
	point.y /= N;
	return point;
}
