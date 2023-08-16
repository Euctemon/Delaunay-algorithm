#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {
	
	std::vector<Point> points = { {0,0}, {1,0},{1,1},{0,1} };

	Canvas canvas(points);
	
	canvas.insertPoint(Point{ 0,0.5 }, true);
	canvas.insertPoint(Point{ 0.5,1 }, true);
	canvas.insertPoint(Point{ 0.5,0.5 }, true);
	canvas.printFaces();

	return 0;
}