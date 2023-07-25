#include<tuple>
#include<iostream>

#include "TrigComputations.h"
#include "DelaunayAlgo.h"


Vertex::Vertex(Point& point) {
	x = point.x;
	y = point.y;
	incident = nullptr;
}

Point Vertex::asPoint() { return Point{ x,y }; }

void Vertex::printCoords() { std::cout << x << '\t' << y << '\n'; }



HalfEdge::HalfEdge(Vertex* originPt) {
	Origin = originPt;
	Next = nullptr;
	Prev = nullptr;
	Triangle = nullptr;
	Twin = nullptr;
}

Vertex* HalfEdge::getOrigin() { return Origin; }

Vertex* HalfEdge::getTarget() { return Twin->getOrigin(); }

HalfEdge* HalfEdge::getNext() { return this->Next; }

HalfEdge* HalfEdge::getPrev() { return this->Prev; }

HalfEdge* HalfEdge::getTwin() { return Twin; };

bool HalfEdge::isBoundary() { return (getTwin()->Triangle == nullptr); }

void HalfEdge::printOrigin() { Origin->printCoords(); };

void HalfEdge::changeorigin(Vertex* vertex) { Origin = vertex; };

void HalfEdge::connectPrevNext(HalfEdge* prev, HalfEdge* next) {
	this->Prev = prev;
	this->Next = next;
}

void HalfEdge::connectTwin(HalfEdge* halfEdge) { this->Twin = halfEdge; }

void HalfEdge::assingFace(Face* face) { this->Triangle = face; }



Face::Face(HalfEdge* boundary) {
	this->boundary = boundary;
	if (boundary) {
		boundary->assingFace(this);
		boundary->getNext()->assingFace(this);
		boundary->getPrev()->assingFace(this);
	}
}

HalfEdge* Face::getEdge() { return boundary; }

std::tuple<Vertex, Vertex, Vertex> Face::getVertices() {
	Vertex* a = boundary->getOrigin();
	Vertex* b = boundary->getNext()->getOrigin();
	Vertex* c = boundary->getPrev()->getOrigin();
	return std::make_tuple(*a, *b, *c);
}

void Face::printVertices() {
	if (this) {
		boundary->printOrigin();
		boundary->getNext()->printOrigin();
		boundary->getPrev()->printOrigin();
	}
	else std::cout << "not found";
}

bool Face::contains(Vertex vertex) {
	auto&& [a, b, c] = this->getVertices();

	return inTriangle(a.asPoint(), b.asPoint(), c.asPoint(), vertex.asPoint());
}



Canvas::Canvas(std::vector<Point> boundary) {
	boundaryVect = boundary;
	std::tuple<Point, Point, Point> trianglePoints = boundingTrianglePoints(boundary);
	makeInitialTriangle(trianglePoints);
}

Face* Canvas::findFace(Vertex vertex) {
	for (auto face : faceVect) {
		if ((*face).contains(vertex)) {
			return face;
		}
	}
	return nullptr;
}

std::tuple<HalfEdge*, HalfEdge*> Canvas::makeTwins(Vertex* left, Vertex* right) {
	HalfEdge* lr = new HalfEdge{ left };
	HalfEdge* rl = new HalfEdge{ right };

	lr->connectTwin(rl);
	rl->connectTwin(lr);

	return std::make_tuple(lr, rl);
}

void Canvas::makeInitialTriangle(std::tuple<Point, Point, Point> trianglePoints) {
	Vertex* a = new Vertex{ std::get<0>(trianglePoints) };
	Vertex* b = new Vertex{ std::get<1>(trianglePoints) };
	Vertex* c = new Vertex{ std::get<2>(trianglePoints) };


	auto&& [ab, ba] = makeTwins(a, b);
	auto&& [ac, ca] = makeTwins(a, c);
	auto&& [bc, cb] = makeTwins(b, c);

	ab->connectPrevNext(ca, bc);
	ba->connectPrevNext(cb, ac);

	ac->connectPrevNext(ba, cb);
	ca->connectPrevNext(bc, ab);

	bc->connectPrevNext(ab, ca);
	cb->connectPrevNext(ac, ba);

	Face* innerFace = new Face{ ab };

	this->verticesVect.assign({ a,b,c });
	this->edgesVect.assign({ ab,ba, ac, ca, bc, cb });
	this->faceVect.assign({ innerFace });
}

void Canvas::swapNecessary(HalfEdge* swapEdge) {
	if (swapEdge->isBoundary()) return;

	Vertex* a = swapEdge->getOrigin();
	Vertex* b = swapEdge->getTarget();
	Vertex* p = swapEdge->getPrev()->getOrigin();
	Vertex* d = swapEdge->getTwin()->getNext()->getTarget();

	if (inCircle(a->asPoint(), b->asPoint(), p->asPoint(), d->asPoint())) {
		flipEdge(swapEdge);
		swapNecessary(swapEdge->getNext());
		swapNecessary(swapEdge->getTwin()->getPrev());
	}
}

void Canvas::printFaces() {
	for (auto& elem : faceVect) {
		elem->printVertices();
		std::cout << '\n';
	}
}

void Canvas::insertInFace(Point point) {
	Vertex* vertex = new Vertex{ point };
	verticesVect.push_back(vertex);

	Face* oldTriangle = findFace(*vertex);
	HalfEdge* ab = oldTriangle->getEdge();
	HalfEdge* next = ab->getNext();
	HalfEdge* prev = ab->getPrev();



	Vertex* a = ab->getOrigin();
	Vertex* b = ab->getTarget();
	Vertex* c = ab->getNext()->getTarget();

	const auto& [ax, xa] = makeTwins(a, vertex);
	const auto& [bx, xb] = makeTwins(b, vertex);
	const auto& [cx, xc] = makeTwins(c, vertex);


	edgesVect.insert(edgesVect.end(), { ax,xa,bx,xb,cx,xc });

	ab->connectPrevNext(xa, bx);
	next->connectPrevNext(xb, cx);
	prev->connectPrevNext(xc, ax);

	ax->connectPrevNext(prev, xc);
	xa->connectPrevNext(ax, prev);

	bx->connectPrevNext(ab, xa);
	xb->connectPrevNext(cx, next);

	cx->connectPrevNext(next, xb);
	xc->connectPrevNext(ax, prev);

	Face* abx = new Face{ ab };
	Face* bcx = new Face{ next };
	Face* cax = new Face{ prev };

	faceVect.insert(faceVect.end(), { abx,bcx,cax });

	deleteFace(oldTriangle);

	swapNecessary(ab);
	swapNecessary(next);
	swapNecessary(prev);
}

void Canvas::deleteFace(Face* face) {
	for (auto elemIt = faceVect.begin(); elemIt < faceVect.end(); elemIt++) {
		if (*elemIt == face) {
			delete* elemIt;
			faceVect.erase(elemIt);
			break;
		}
	}
}

void Canvas::flipEdge(HalfEdge* edgeToSwap) {
	HalfEdge* ac = edgeToSwap;
	HalfEdge* ca = ac->getTwin();

	HalfEdge* ab = ca->getNext();
	HalfEdge* bc = ab->getNext();
	HalfEdge* cb = ab->getTwin();
	HalfEdge* cd = ac->getNext();
	HalfEdge* da = cd->getNext();
	HalfEdge* ad = cd->getTwin();

	Vertex* b = bc->getOrigin();
	Vertex* d = da->getOrigin();

	ca->changeorigin(d);
	ac->changeorigin(b);

	bc->connectTwin(da);
	da->connectTwin(bc);

	ca->connectTwin(ad);
	ad->connectTwin(ca);

	ac->connectTwin(cb);
	cb->connectTwin(ac);
}