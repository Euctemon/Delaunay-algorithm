#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {

	std::vector<Point> points = { Point{1,1}, Point{7,0}, Point{5,5} };
	Canvas canvas(points);

	return 0;
}
