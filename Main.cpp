#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {
	Point a{ 0,0 };
	Point b{ 0,3 };
	Point c{ 2,4 };
	Point d{ 4,2 };
	Point e{ 3,0 };
	
	std::vector<Point> points = { a,b,c,d,e };
	Canvas canvas(points);
	canvas.Ruppert();
	
	return 0;
}
