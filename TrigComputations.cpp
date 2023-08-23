#include<cstdlib>
#include<tuple>
#include<vector>
#include<algorithm>
#include<iostream>
#include<optional>

#include "TrigComputations.h"

Point Point::operator-(const Point& other)
{
	return Point{ x - other.x,y - other.y };
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

bool inCircSegment(Point a, Point b, Point c, Point d) {
	Point vec_da = a - d;
	Point vec_db = b - d;
	Point vec_dc = c - d;

	double det = (vec_da.x * vec_db.y - vec_db.x * vec_da.y) * dot(vec_dc,vec_dc) +
				 (vec_db.x * vec_dc.y - vec_db.y * vec_dc.x) * dot(vec_da,vec_da) +
				 (vec_dc.x * vec_da.y - vec_dc.y * vec_da.x) * dot(vec_db,vec_db);

	if (det > 0) return true;
	else return false;
}

bool inCircle(Point a, Point b, Point c, Point d) {
	int sum = orientedTriangle(a, b, d) + orientedTriangle(b, c, d) + orientedTriangle(c, a, d);

	if (sum < 1) return false;
	else if (sum > 1) return true;
	else return inCircSegment(a, b, c, d);
}

bool hasBadAngle(Point a, Point b, Point c) {
	Point circumcenter = getCircumcenter(a, b, c);
	Point circumradius = a - circumcenter;
	Point vec_ab = b - a;
	Point vec_bc = c - b;
	Point vec_ca = a - c;
	
	auto smallestSide = std::min({ vec_ab,vec_bc,vec_ca }, [](Point a, Point b) {return (dot(a,a)< dot(b,b)); });
	float ratio = dot(circumradius, circumradius) / dot(smallestSide, smallestSide);

	return (ratio < 2.0 ? false : true);
}

bool hasBadArea(Point a, Point b, Point c)
{
	Point vec_ba = a - b;
	Point vec_ca = a - c;
	double maxArea = 0.2;
	double area = abs(vec_ba.x * vec_ca.y - vec_ba.y * vec_ca.x)/2.0;

	return (area > maxArea);
}

bool isNearHalfEdge(Point origin, Point target, Point a) {
	Point halfedgeMid{ (origin.x + target.x) / 2.0,(origin.y + target.y) / 2.0 };
	bool isNear = dot(origin - halfedgeMid, origin - halfedgeMid) >= dot(a - halfedgeMid, a - halfedgeMid);
	bool isAntiClockwise = (orientedTriangle(origin, target, a) > -1);

	return (isNear && isAntiClockwise);
}

int orientedTriangle(Point a, Point b, Point c) {
	Point vec_ca = a - c;
	Point vec_cb = b - c;
	double det = vec_ca.x * vec_cb.y - vec_ca.y * vec_cb.x;

	if (std::abs(det) < 1E-10) return 0;
	else if (det > 0) return 1;
	else return -1;
}

double dot(Point a, Point b) {
	return (a.x * b.x + a.y * b.y);
}

Point getCircumcenter(Point a, Point b, Point c) {
	double sX = dot(a,a) * (b.y - c.y) + dot(b,b) * (c.y - a.y) + dot(c,c) * (a.y - b.y);
	double sY = dot(a,a) * (c.x - b.x) + dot(b,b) * (a.x - c.x) + dot(c,c) * (b.x - a.x);
	double scale = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));

	return Point{ sX / scale,sY / scale };
}

Point getMidpoint(Point a, Point b) {
	double xMid = (a.x + b.x) / 2.0;
	double yMid = (a.y + b.y) / 2.0;
	
	return Point(xMid,yMid);
}

std::tuple<Point, Point, Point> boundingTrianglePoints(std::vector<Point>& boundaryPoints) {
	Point a{};
	Point b{};
	Point c{};

	auto&& [xMinIter, xMaxIter] = std::minmax_element(boundaryPoints.begin(), boundaryPoints.end(), [](Point const& a, Point const& b) {return a.x < b.x; });
	auto&& [yMinIter, yMaxIter] = std::minmax_element(boundaryPoints.begin(), boundaryPoints.end(), [](Point const& a, Point const& b) {return a.y < b.y; });

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
