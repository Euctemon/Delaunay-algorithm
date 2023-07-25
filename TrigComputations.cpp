#include<cstdlib>
#include<tuple>
#include<vector>
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

bool inTriangle(Point a, Point b, Point c, Point d) {
	int sum = orientedTriangle(a, b, d) + orientedTriangle(b, c, d) + orientedTriangle(c, a, d);

	if (sum == 3) return true;
	else return false;
}

bool inCircle(Point a, Point b, Point c, Point d) {
	int sum = orientedTriangle(a, b, d) + orientedTriangle(b, c, d) + orientedTriangle(c, a, d);

	if (sum < 1) return false;
	else if (sum > 1) return true;
	else return inCircSegment(a, b, c, d);
}

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints) {
	Point a = { -40.0,-10.0 };
	Point b = { 40.0,-10.0 };
	Point c = { 0.0,50.0 };
	return std::make_tuple(a, b, c);
}