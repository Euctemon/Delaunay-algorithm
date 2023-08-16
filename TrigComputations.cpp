#include<cstdlib>
#include<tuple>
#include<vector>
#include<algorithm>
#include<iostream>
#include<optional>

#include "TrigComputations.h"

int orientedTriangle(Point a, Point b, Point c) {
	Point vec1 = { a.x - c.x , a.y - c.y };
	Point vec2 = { b.x - c.x , b.y - c.y };
	double det = vec1.x * vec2.y - vec1.y * vec2.x;

	if (std::abs(det) < 1E-10) return 0;
	else if (det > 0) return 1;
	else return -1;
}

bool inCircSegment(Point a, Point b, Point c, Point d) {
	Point vec1 = { a.x - d.x,a.y - d.y };
	Point vec2 = { b.x - d.x,b.y - d.y };
	Point vec3 = { c.x - d.x, c.y - d.y };
	double vec1mag = vec1.x * vec1.x + vec1.y * vec1.y;
	double vec2mag = vec2.x * vec2.x + vec2.y * vec2.y;
	double vec3mag = vec3.x * vec3.x + vec3.y * vec3.y;

	double det = (vec1.x * vec2.y - vec2.x * vec1.y) * vec3mag +
		(vec2.x * vec3.y - vec2.y * vec3.x) * vec1mag +
		(vec3.x * vec1.y - vec3.y * vec1.x) * vec2mag;

	if (det > 0) return true;
	else return false;
}

pointPos inTriangle(Point a, Point b, Point c, Point d) {
	int sum = orientedTriangle(a, b, d) + orientedTriangle(b, c, d) + orientedTriangle(c, a, d);

	switch (sum) {
	case 2:
		return BOUNDARY;		// on triangle's edge
	case 3:
		return INSIDE;		// in triangle
	default:
		return OUTSIDE;		// outside triangle
	}
}

bool inCircle(Point a, Point b, Point c, Point d) {
	int sum = orientedTriangle(a, b, d) + orientedTriangle(b, c, d) + orientedTriangle(c, a, d);

	if (sum < 1) return false;
	else if (sum > 1) return true;
	else return inCircSegment(a, b, c, d);
}

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints) {
	Point a{};
	Point b{};
	Point c{};

	const auto& [xMinIter, xMaxIter] = std::minmax_element(boundaryPoints.begin(), boundaryPoints.end(), [](Point const& a, Point const& b) {return a.x < b.x; });
	const auto& [yMinIter, yMaxIter] = std::minmax_element(boundaryPoints.begin(), boundaryPoints.end(), [](Point const& a, Point const& b) {return a.y < b.y; });

	double xMin = xMinIter->x - 1.0;
	double xMax = xMaxIter->x + 1.0;
	double yMin = yMinIter->y - 1.0;
	double yMax = yMaxIter->y + 1.0;

	if (xMax - xMin > yMax - yMin) {
		a = { xMin, 2.0 * yMin - yMax };
		b = { 2.0 * xMax, 0.5 * (yMax + yMin) };
		c = { xMin, 2.0 * yMax - yMin };
	}
	else {
		a = { 2.0 * xMin - xMax,yMin };
		b = { 2.0 * xMax - xMin,yMin };
		c = { 0.5 * (xMax + xMin),2.0 * yMax };
	}

	return std::make_tuple(a, b, c);
}

std::vector<Point> makeStructuredPoints(std::vector<Point>& boundaryPoints, float spacing) {
	std::vector<Point> vec{};
	const auto& [xMinIter, xMaxIter] = std::minmax_element(boundaryPoints.begin(), boundaryPoints.end(), [](Point const& a, Point const& b) {return a.x < b.x; });
	const auto& [yMinIter, yMaxIter] = std::minmax_element(boundaryPoints.begin(), boundaryPoints.end(), [](Point const& a, Point const& b) {return a.y < b.y; });

	int numXpoints = int((xMaxIter->x - xMinIter->x) / spacing) + 1;
	int numYpoints = int((yMaxIter->y - yMinIter->y) / spacing);

	for (int i = 0; i < numXpoints; i++) {
		for (int j = 0; j < numYpoints; j++) {
			if (j % 2 == 0) vec.push_back({ xMinIter->x + i * spacing, yMinIter->y + j * spacing });
			else vec.push_back({ xMinIter->x + i * spacing - spacing/2, yMinIter->y + j * spacing });
		}
	}
	return vec;
}

std::optional<std::vector<Point>> makeBoundaryPoints(Point a, Point b, float spacing)
{
	std::vector<Point> points{};
	float euclid = pow(pow((a.x - b.x), 2.0) + pow((a.y - b.y), 2.0), 0.5);
	float abVectX = b.x - a.x;
	float abVectY = b.y - a.y;
	int ratio = int(euclid / spacing);

	for (int i = 1; i< ratio; i++) {
		points.push_back({ a.x + i * (abVectX / ratio), a.y + i * (abVectY / ratio) });
	}
	return points;
}
