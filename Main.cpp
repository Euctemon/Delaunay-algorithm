#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {
	
	std::vector<Point> points = { {3,3}, {3,-3},{-3,-3},{-3,3}};

	Canvas canvas(points);
	canvas.printFaces();


	return 0;
}