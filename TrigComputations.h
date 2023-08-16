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

Point getCircumcenter(Point a, Point b, Point c);

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints);

std::vector<Point> makeStructuredPoints(std::vector<Point>& boundaryPoints, float spacing);

std::optional<std::vector<Point>> makeBoundaryPoints(Point a, Point b, float spacing);