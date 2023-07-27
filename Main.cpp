#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {

	Point a{ 18.0,0.0 };
	Point b{ 0.0,-10.0 };
	Point c{ 0.0,12.0 };
	Point d{ 1.0,3.0 };
	Point e{ -5.0,2.0 };
	Point f{ -14.0,5.0 };
	
	std::vector<Point> points = { a,b,c,d,e};

	Canvas canvas(points);
	


	return 0;
}