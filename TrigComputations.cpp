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
		return pointPos::BOUNDARY;		// on triangle's edge
	case 3:
		return pointPos::INSIDE;		// in triangle
	default:
		return pointPos::OUTSIDE;		// outside triangle
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

bool inCircle(Point a, Point b, Point c, Point point) {
	int sum = orientedTriangle(a, b, point) + orientedTriangle(b, c, point) + orientedTriangle(c, a, point);

	if (sum < 1) return false;
	else if (sum > 1) return true;
	else return inCircSegment(a, b, c, point);
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

std::tuple<Point, Point, Point> getBoundingPoints(std::vector<Point>& points) {
	auto&& [xMin, xMax] = std::minmax_element(points.begin(), points.end(), [](Point const& a, Point const& b) {return a.x < b.x; });
	auto&& [yMin, yMax] = std::minmax_element(points.begin(), points.end(), [](Point const& a, Point const& b) {return a.y < b.y; });

	Point center{ (xMin->x + xMax->x) / 2.0, (yMin->y + yMax->y) / 2.0 };
	
	double xDiff = abs(xMax->x - center.x) + 1;
	double yDiff = abs(yMax->y - center.y) + 1;

	Point a{ center.x,center.y + 2 * yDiff };
	Point b{ center.x - 2 * xDiff,center.y - 2 * yDiff };
	Point c{ center.x + 2 * xDiff,center.y - 2 * yDiff };

	return std::make_tuple(a, b, c);
}
