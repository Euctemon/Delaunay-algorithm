#include<tuple>
#include<iostream>
#include<algorithm>
#include<optional>
#include<functional>

#include "TrigComputations.h"
#include "DelaunayAlgo.h"


// VRCHOLY

Vertex::Vertex(Point& point) {
	x = point.x;
	y = point.y;
	Incident = nullptr;
}

Point Vertex::asPoint() { return Point{ x,y }; }

HalfEdge* Vertex::getEdge() { return Incident; }

void Vertex::assignEdge(HalfEdge* halfedge) { Incident = halfedge; }


// HRANY

HalfEdge::HalfEdge(Vertex* origin) {
	Origin = origin;
	Next = nullptr;
	Prev = nullptr;
	Triangle = nullptr;
	Twin = nullptr;

	origin->assignEdge(this);
}

Vertex* HalfEdge::getOrigin() { return Origin; }

Vertex* HalfEdge::getTarget() { return Next->getOrigin(); }

HalfEdge* HalfEdge::getNext() { return Next; }

HalfEdge* HalfEdge::getPrev() { return Prev; }

HalfEdge* HalfEdge::getTwin() { return Twin; }

Face* HalfEdge::getFace() { return Triangle; }

bool HalfEdge::isBoundary() { return (getTwin() == nullptr); }

void HalfEdge::changeOrigin(Vertex* vertex) { Origin = vertex; };

void HalfEdge::assignTwin(HalfEdge* halfEdge) { Twin = halfEdge; }

void HalfEdge::assingTriangle(Face* triangle) { Triangle = triangle; }

void connectTriangle(HalfEdge* first, HalfEdge* second, HalfEdge* third) {
	first->Prev = third;
	first->Next = second;

	second->Prev = first;
	second->Next = third;

	third->Prev = second;
	third->Next = first;
}

void connectTwins(HalfEdge* first, HalfEdge* second) {
	first->Twin = second;
	second->Twin = first;
}

// TROJÚHELNÍKY

Face::Face(HalfEdge* boundary) {
	Boundary = boundary;
	if (boundary) {
		boundary->assingTriangle(this);
		boundary->getNext()->assingTriangle(this);
		boundary->getPrev()->assingTriangle(this);
	}
}

HalfEdge* Face::getEdge() { return Boundary; }

HalfEdge* Face::edgeContainingPoint(Point point) {
	HalfEdge* ab = getEdge();
	HalfEdge* bc = ab->getNext();
	HalfEdge* ca = ab->getPrev();

	if (orientedTriangle(ab->getOrigin()->asPoint(), ab->getTarget()->asPoint(), point) == 0) return ab;
	if (orientedTriangle(bc->getOrigin()->asPoint(), bc->getTarget()->asPoint(), point) == 0) return bc;
	else return ca;
}

std::tuple<Point, Point, Point> Face::getVertices() {
	Point a = Boundary->getOrigin()->asPoint();
	Point b = Boundary->getNext()->getOrigin()->asPoint();
	Point c = Boundary->getPrev()->getOrigin()->asPoint();
	return std::make_tuple(a, b, c);
}

std::tuple<HalfEdge*, HalfEdge*, HalfEdge*> Face::getEdges() {
	return std::make_tuple(Boundary, Boundary->getNext(), Boundary->getPrev());
}

bool Face::containsAtleastOne(std::tuple<Point, Point, Point> boudingPoints) {
	auto&& [a,b,c] = getVertices();
	auto&& [A, B, C] = boudingPoints;

	if (a == A || a == B || a == C) return true;
	if (b == A || b == B || b == C) return true;
	if (c == A || c == B || c == C) return true;

	return false;
}

std::optional<std::variant<Face*, HalfEdge*>> Face::contains(Point point) {
	auto&& [a, b, c] = getVertices();
	pointPos position = inTriangle(a, b, c, point);

	switch (position) {
	case pointPos::INSIDE:
		return this;
	case pointPos::BOUNDARY:
		return edgeContainingPoint(point);
	case pointPos::OUTSIDE:
		return std::nullopt;
	}
}

// PLÁTNO

void Canvas::eraseEdge(HalfEdge* halfedge) {
	auto edgeIt = std::find(begin(edgeVect), end(edgeVect), halfedge);
	if (!halfedge->isBoundary()) halfedge->getTwin()->assignTwin(nullptr);
	
	delete* edgeIt;
	edgeVect.erase(edgeIt);
}

bool Canvas::cleanTriangle(Face* triangle, std::tuple<Point, Point, Point> boundingPoints) {
	bool shouldDelete{ false };
	if (triangle->containsAtleastOne(boundingPoints)) {
		auto&& [ab, bc, ca] = triangle->getEdges();

		eraseEdge(ab);
		eraseEdge(bc);
		eraseEdge(ca);

		delete triangle;
		shouldDelete = true;
	}
	return shouldDelete;
}

bool Canvas::cleanVertex(Vertex* vertex, std::tuple<Point, Point, Point> boundingPoints) {
	bool shouldDelete{ false };
	auto&& [a, b, c] = boundingPoints;
	Point point = vertex->asPoint();

	if (point == a || point == b || point == c) {
		delete vertex;
		shouldDelete = true;
	}
	return shouldDelete;
}

void Canvas::insertInFace(Face* face, Point& point) {
	HalfEdge* ab = face->getEdge();
	HalfEdge* bc = ab->getNext();
	HalfEdge* ca = ab->getPrev();

	Vertex* a = ab->getOrigin();
	Vertex* b = bc->getOrigin();
	Vertex* c = ca->getOrigin();

	Vertex* vertex = new Vertex{ point };
	
	HalfEdge* ax = new HalfEdge{ a };
	HalfEdge* bx = new HalfEdge{ b };
	HalfEdge* cx = new HalfEdge{ c };
	
	HalfEdge* xa = new HalfEdge{ vertex };
	HalfEdge* xb = new HalfEdge{ vertex };
	HalfEdge* xc = new HalfEdge{ vertex };

	vertex->assignEdge(xa);

	connectTriangle(ab, bx, xa);
	connectTriangle(bc, cx, xb);
	connectTriangle(ca, ax, xc);

	connectTwins(ax, xa);
	connectTwins(bx, xb);
	connectTwins(cx, xc);

	Face* abx = new Face{ ab };
	Face* bcx = new Face{ bc };
	Face* cax = new Face{ ca };

	vertexVect.push_back(vertex);
	edgeVect.insert(edgeVect.end(), { ax,xa,bx,xb,cx,xc });
	triangleVect.insert(triangleVect.end(), { abx,bcx,cax });

	auto face_iter = std::find(begin(triangleVect), end(triangleVect), face);
	delete* face_iter;
	triangleVect.erase(face_iter);
	
	swapNecessary(xa->getNext());
	swapNecessary(xb->getNext());
	swapNecessary(xc->getNext());
	
	// fliEdge uvnitr swap meni strukturu trojuhelniku, next pro ab uz neni validni
}

void Canvas::insertInInnerEdge(HalfEdge* halfedge, Point& point) {
	HalfEdge* ac = halfedge;
	HalfEdge* ca = halfedge->getTwin();
	HalfEdge* xd;
	HalfEdge* bx;

	Vertex* vertex = new Vertex{ point };

	HalfEdge* bc = new HalfEdge{ca->getPrev()->getOrigin()};
	HalfEdge* cx = new HalfEdge{ca->getOrigin()};
	HalfEdge* xb = new HalfEdge{ vertex };

	HalfEdge* cd = new HalfEdge{ac->getTarget()};
	HalfEdge* dx = new HalfEdge{ac->getPrev()->getOrigin()};
	HalfEdge* xc = new HalfEdge{ vertex };

	connectTriangle(bc, cx, xb);
	connectTriangle(cd, dx, xc);

	Face* bcx = new Face{ bc };
	Face* cdx = new Face{ cd };

	ca->getPrev()->isBoundary() ? bc->assignTwin(nullptr) : connectTwins(bc, ca->getPrev()->getTwin());
	ac->getNext()->isBoundary() ? cd->assignTwin(nullptr) : connectTwins(cd, ac->getNext()->getTwin());

	ca->changeOrigin(vertex);
	ac->getNext()->changeOrigin(vertex);
	
	xd = ac->getNext();
	bx = ca->getPrev();
	
	connectTwins(cx, xc);
	connectTwins(bx, xb);
	connectTwins(xb, dx);

	vertexVect.push_back(vertex);
	edgeVect.insert(edgeVect.end(), { bc,cx,xb,cd,dx,xc });
	triangleVect.insert(triangleVect.end(), { bcx,cdx });

	swapNecessary(ca->getNext());
	swapNecessary(xb->getNext());
	swapNecessary(xc->getNext());
	swapNecessary(xd->getNext());
}

void Canvas::insertInBoundaryEdge(HalfEdge* halfedge, Point& point) {
	HalfEdge* bc = halfedge;

	Vertex* vertex = new Vertex{ point };

	HalfEdge* ab = new HalfEdge{ bc->getPrev()->getOrigin()};
	HalfEdge* bx = new HalfEdge{ bc->getOrigin()};
	HalfEdge* xa = new HalfEdge{ vertex };
	
	connectTriangle(ab, bx, xa);
	
	Face* xbc = new Face{ ab };

	bc->getPrev()->isBoundary() ? ab->assignTwin(nullptr) : connectTwins(ab, bc->getPrev()->getTwin());
	bc->changeOrigin(vertex);

	connectTwins(bc->getPrev(), xa);

	vertexVect.push_back(vertex);
	edgeVect.insert(edgeVect.end(), { ab,bx,xa });
	triangleVect.push_back(xbc);

	swapNecessary(bc->getNext());
	swapNecessary(xa->getNext());
}

void Canvas::makeEnclosingTriangle(std::tuple<Point,Point,Point> boundingPoints) {
	auto&& [a, b, c] = boundingPoints;
	
	Vertex* A = new Vertex{ a };
	Vertex* B = new Vertex{ b };
	Vertex* C = new Vertex{ c };

	HalfEdge* AB = new HalfEdge{ A };
	HalfEdge* BC = new HalfEdge{ B };
	HalfEdge* CA = new HalfEdge{ C };
	
	connectTriangle(AB, BC, CA);

	Face* ABC = new Face{ AB };
	
	vertexVect.assign({ A,B,C });
	edgeVect.assign({ AB,BC,CA });
	triangleVect.push_back(ABC);
}

void Canvas::populateCanvas() {
	for (auto& point : inputPoints) { insertPoint(point); }
}

void Canvas::removeEnclosingTriangle(std::tuple<Point, Point, Point> boundingPoints) {
	auto decideTriangle = [&](Face* triangle) {return cleanTriangle(triangle, boundingPoints); };
	auto decideVertex = [&](Vertex* vertex) {return cleanVertex(vertex, boundingPoints); };

	std::erase_if(triangleVect, decideTriangle);
	std::erase_if(vertexVect, decideVertex);
}

void Canvas::flipEdge(HalfEdge* edgeToSwap) {
	HalfEdge* ab = edgeToSwap;
	HalfEdge* ba = ab->getTwin();

	HalfEdge* cb = ba->getPrev();
	HalfEdge* bc = cb->getTwin();

	HalfEdge* da = ab->getPrev();
	HalfEdge* ad = da->getTwin();

	ba->changeOrigin(da->getOrigin());
	ab->changeOrigin(cb->getOrigin());

	connectTwins(da, cb);

	(ba == nullptr) ? ad->assignTwin(nullptr) : connectTwins(ba, ad);
	(bc == nullptr) ? ab->assignTwin(nullptr) : connectTwins(ab, bc);
	
}

void Canvas::swapNecessary(HalfEdge* halfedge) {
	if (halfedge->isBoundary()) return;

	HalfEdge* left = halfedge->getNext()->getTwin();
	HalfEdge* right = halfedge->getPrev()->getTwin();

	Vertex* a = halfedge->getOrigin();
	Vertex* b = halfedge->getTarget();
	Vertex* p = halfedge->getPrev()->getOrigin();
	Vertex* d = halfedge->getTwin()->getNext()->getTarget();

	if (inCircle(a->asPoint(), b->asPoint(), p->asPoint(), d->asPoint())) {
		// flipedge meni strukturu trojuhelniku, next uz neni validni
		flipEdge(halfedge);
		swapNecessary(left->getTwin()->getPrev());
		swapNecessary(right->getTwin()->getNext());
	}
}

Canvas::Canvas(std::vector<Point> points) : insertFunctor(*this) {
	inputPoints = points;
	auto boundingPoints = getBoundingPoints(points);

	makeEnclosingTriangle(boundingPoints);
	populateCanvas();
	removeEnclosingTriangle(boundingPoints);
}

void Canvas::printByEdges() {
	Point a{};
	Point b{};
	Point c{};
	
	for (auto& edge : edgeVect) {
		a = edge->getOrigin()->asPoint();
		b = edge->getNext()->getOrigin()->asPoint();
		c = edge->getPrev()->getOrigin()->asPoint();
		std::cout << a.x << '\t' << a.y << '\n';
		std::cout << b.x << '\t' << b.y << '\n';
		std::cout << c.x << '\t' << c.y << '\n';

		std::cout << "\n-----------\n";
	}
}

void Canvas::insertPoint(Point point) {
	for (auto triangle : triangleVect) {
		auto maybeContains = triangle->contains(point);
		
		if (maybeContains.has_value()) {
			insertFunctor.point = point;
			std::visit(insertFunctor, maybeContains.value());
			break;
		}
	}
}


