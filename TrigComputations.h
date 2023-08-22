#pragma once

#include<vector>

enum pointPos { INSIDE, OUTSIDE, BOUNDARY };

struct Point {
	double x;
	double y;

	bool operator==(const Point& other) const = default;

	Point operator-(const Point& other);

};

double dot(Point a, Point b);

int orientedTriangle(Point a, Point b, Point c);

bool inCircSegment(Point a, Point b, Point c, Point d);

pointPos inTriangle(Point a, Point b, Point c, Point d);

bool inCircle(Point a, Point b, Point c, Point d);

bool hasBadAngle(Point a, Point b, Point c);

bool hasBadArea(Point a, Point b, Point c);

bool isNearHalfEdge(Point origin, Point target, Point a);

Point getCircumcenter(Point a, Point b, Point c);

Point getMidpoint(Point a, Point b);

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints);