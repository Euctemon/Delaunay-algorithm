#include<cstdlib>
#include<tuple>
#include<vector>
#include<algorithm>
#include<iostream>
#include<optional>

#include "TrigComputations.h"

double dot(Point a, Point b)
{
	return (a.x*b.x+a.y*b.y);
}

Point Point::operator-(const Point& other)
{
	return Point{ x - other.x,y - other.y };
}

int orientedTriangle(Point a, Point b, Point c) {
	Point vec1 = a - c;		// { a.x - c.x , a.y - c.y }
	Point vec2 = b - c;		// { b.x - c.x, b.y - c.y };
	double det = vec1.x * vec2.y - vec1.y * vec2.x;

	if (std::abs(det) < 1E-10) return 0;
	else if (det > 0) return 1;
	else return -1;
}

bool inCircSegment(Point a, Point b, Point c, Point d) {
	Point vec1 = a - d;		// { a.x - d.x, a.y - d.y };
	Point vec2 = b - d;		// { b.x - d.x, b.y - d.y };
	Point vec3 = c - d;		// { c.x - d.x, c.y - d.y };

	double det = (vec1.x * vec2.y - vec2.x * vec1.y) * dot(vec3,vec3) +
				 (vec2.x * vec3.y - vec2.y * vec3.x) * dot(vec1,vec1) +
				 (vec3.x * vec1.y - vec3.y * vec1.x) * dot(vec2,vec2);

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

bool hasBadAngle(Point a, Point b, Point c)
{
	/*double cutoffAngle = 0.88;
	Point ab = b - a;
	Point bc = c - b;
	Point ca = a - c;

	double alpha = dot(ca, ab) * dot(ca, ab) / (dot(ca, ca) * dot(ab, ab));
	double beta = dot(ab, bc) * dot(ab, bc) / (dot(ab, ab) * dot(bc, bc));
	double gamma = dot(bc, ca) * dot(bc, ca) / (dot(bc, bc) * dot(ca, ca));
	
	bool smallangle = (alpha > cutoffAngle) || (beta > cutoffAngle) || (gamma > cutoffAngle);*/
	Point circumcenter = getCircumcenter(a, b, c);
	Point circumradius = a - circumcenter;
	Point ab = b - a;
	Point bc = c - b;
	Point ca = a - c;
	auto smallestSide = std::min({ ab,bc,ca }, [](Point a, Point b) {return (dot(a,a)< dot(b,b)); });
	float ratio = dot(circumradius, circumradius) / dot(smallestSide, smallestSide);

	return (ratio < 2.0 ? false : true);
}

bool hasBadArea(Point a, Point b, Point c)
{
	Point ba = a - b;
	Point ca = a - c;
	double maxArea = 0.15;
	double area = abs(ba.x * ca.y - ba.y * ca.x)/2.0;

	return (area > maxArea);
}

bool isNearHalfEdge(Point origin, Point target, Point a)
{
	Point halfedgeMid{ (origin.x + target.x) / 2.0,(origin.y + target.y) / 2.0 };
	bool isNear = dot(origin - halfedgeMid, origin - halfedgeMid) >= dot(a - halfedgeMid, a - halfedgeMid);
	bool isAntiClockwise = (orientedTriangle(origin, target, a) > -1);

	return (isNear && isAntiClockwise);
}


Point getCircumcenter(Point a, Point b, Point c)
{
	double sX = dot(a,a) * (b.y - c.y) + dot(b,b) * (c.y - a.y) + dot(c,c) * (a.y - b.y);
	double sY = dot(a,a) * (c.x - b.x) + dot(b,b) * (a.x - c.x) + dot(c,c) * (b.x - a.x);
	double scale = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));

	return Point{ sX / scale,sY / scale };
}

Point getMidpoint(Point a, Point b)
{
	double xMid = (a.x + b.x) / 2.0;
	double ymid = (a.y + b.y) / 2.0;
	return Point(xMid,ymid);
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
