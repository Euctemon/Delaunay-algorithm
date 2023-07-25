#pragma once

#include<vector>

struct Point {
	double x;
	double y;
};

int orientedTriangle(Point a, Point b, Point c);

bool inCircSegment(Point a, Point b, Point c, Point d);

bool inTriangle(Point a, Point b, Point c, Point d);

bool inCircle(Point a, Point b, Point c, Point d);

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints);
