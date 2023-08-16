#include<iostream>
#include<vector>
#include "DelaunayAlgo.h"

int main() {
	
	std::vector<Point> points = { {0,0}, {1,0},{1,1},{0,1} };

	//Canvas canvas(points);
	Point a = getCircumcenter(Point{ 2,2 }, Point{ 5,1 }, Point{ 6,2 });
	std::cout << a.x << '\t' << a.y << std::endl;

	Point b = getCircumcenter(Point{ 5,1 }, Point{ 2,2 }, Point{ 6,2 });
	std::cout << b.x << '\t' << b.y << std::endl;

	Point c = getCircumcenter(Point{ 2,2 }, Point{ 6,2 }, Point{ 5,1 });
	std::cout << c.x << '\t' << c.y << std::endl;


	return 0;
}