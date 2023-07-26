#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {

	Point a{ 18.0,0.0 };
	Point b{ 0.0,-10.0 };
	Point c{ 0.0,12.0 };
	Point d{ 0.0,6.0 };
	
	std::vector<Point> points = { a,b,c,d };

	Canvas canvas(points);

	canvas.printFaces();
	std::cout << "\n -------------- \n";
	canvas.insertInFace(a);
	canvas.printFaces();
	std::cout << "\n -------------- \n";
	canvas.insertInFace(b);
	canvas.printFaces();
	std::cout << "\n -------------- \n";
	canvas.insertInFace(c);
	canvas.printFaces();
	std::cout << "\n -------------- \n";
	canvas.insertInFace(d);
	canvas.printFaces();
	


	return 0;
}