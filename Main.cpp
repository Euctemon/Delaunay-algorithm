#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {

	Point a{ 2.0,2.0 };
	Point b{ 10.0,2.0 };
	Point c{ 5.0,8.0 };

	Point d{ 0.0,10.0 };
	Point e{ 20.0,10.0 };
	Point f{ 10.0,0.0 };

	std::vector<Point> points = { a,b,c };

	Canvas canvas(points);

	canvas.printFaces();

	std::cout << "---------------\n";

	canvas.insertInFace(d);
	canvas.insertInFace(e);
	canvas.insertInFace(f);

	canvas.printFaces();


	return 0;
}