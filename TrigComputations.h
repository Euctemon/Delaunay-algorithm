#pragma once

#include<vector>

enum class pointPos { INSIDE, OUTSIDE, BOUNDARY };

struct Point {
	double x;
	double y;

	bool operator==(const Point& other) const = default;

	Point operator-(const Point& other);

};

pointPos inTriangle(Point a, Point b, Point c, Point d);

bool inCircSegment(Point a, Point b, Point c, Point d);

bool inCircle(Point a, Point b, Point c, Point d);

int orientedTriangle(Point a, Point b, Point c);

double dot(Point a, Point b);

std::tuple<Point, Point, Point> getBoundingPoints(std::vector<Point>& boundaryPoints);