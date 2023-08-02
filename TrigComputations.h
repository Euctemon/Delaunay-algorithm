#pragma once

#include<vector>

enum pointPos { INSIDE, OUTSIDE, BOUNDARY };

struct Point {
	double x;
	double y;

	bool operator==(const Point& other) const = default;
};

int orientedTriangle(Point a, Point b, Point c);

bool inCircSegment(Point a, Point b, Point c, Point d);

pointPos inTriangle(Point a, Point b, Point c, Point d);

bool inCircle(Point a, Point b, Point c, Point d);

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints);
