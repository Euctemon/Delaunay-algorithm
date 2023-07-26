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

void Vertex::assignEdge(HalfEdge* halfedge) { incident = halfedge; }

HalfEdge* Vertex::leftBoundaryEdge(HalfEdge* halfedge)
{
	HalfEdge* boundaryedge = halfedge;
	if (!boundaryedge->isBoundary()) {
		boundaryedge = boundaryedge->getPrev()->getTwin();
	}
	return boundaryedge;
}



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

bool HalfEdge::isBoundary() { return (Triangle == nullptr); }

void HalfEdge::printOrigin() { Origin->printCoords(); };

void HalfEdge::changeorigin(Vertex* vertex) { Origin = vertex; };

void HalfEdge::assignPrevNext(HalfEdge* prev, HalfEdge* next) {
	this->Prev = prev;
	this->Next = next;
}

void HalfEdge::assignTwin(HalfEdge* halfEdge) { this->Twin = halfEdge; }

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
	makeEnclosingTriangle(trianglePoints);
	//populateCanvas();
}

Face* Canvas::findFace(Vertex vertex) {					// rozmyslet si, co se stane, pokud nenajdu trojuhelnik - treba bod lezi na hranici
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

	lr->assignTwin(rl);
	rl->assignTwin(lr);

	return std::make_tuple(lr, rl);
}

void Canvas::makeEnclosingTriangle(std::tuple<Point, Point, Point> trianglePoints) {
	Vertex* a = new Vertex{ std::get<0>(trianglePoints) };
	Vertex* b = new Vertex{ std::get<1>(trianglePoints) };
	Vertex* c = new Vertex{ std::get<2>(trianglePoints) };


	auto&& [ab, ba] = makeTwins(a, b);
	auto&& [ac, ca] = makeTwins(a, c);
	auto&& [bc, cb] = makeTwins(b, c);

	ab->assignPrevNext(ca, bc);
	ba->assignPrevNext(cb, ac);

	ac->assignPrevNext(ba, cb);
	ca->assignPrevNext(bc, ab);

	bc->assignPrevNext(ab, ca);
	cb->assignPrevNext(ac, ba);

	Face* innerFace = new Face{ ab };
	ab->assingFace(innerFace);
	bc->assingFace(innerFace);
	ca->assingFace(innerFace);

	a->assignEdge(ac);
	b->assignEdge(ba);
	c->assignEdge(cb);

	this->verticesVect.assign({ a,b,c });
	this->edgesVect.assign({ ab,ba, ac, ca, bc, cb });
	this->faceVect.assign({ innerFace });
}

void Canvas::swapNecessary(HalfEdge* swapEdge) {
	if (swapEdge->getTwin()->isBoundary()) return;

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
	Vertex* b = next->getOrigin();
	Vertex* c = prev->getOrigin();

	const auto& [ax, xa] = makeTwins(a, vertex);
	const auto& [bx, xb] = makeTwins(b, vertex);
	const auto& [cx, xc] = makeTwins(c, vertex);

	vertex->assignEdge(xa);
	edgesVect.insert(edgesVect.end(), { ax,xa,bx,xb,cx,xc });

	ab->assignPrevNext(xa, bx);
	next->assignPrevNext(xb, cx);
	prev->assignPrevNext(xc, ax);

	ax->assignPrevNext(prev, xc);
	xa->assignPrevNext(ax, prev);

	bx->assignPrevNext(ab, xa);
	xb->assignPrevNext(cx, next);

	cx->assignPrevNext(next, xb);
	xc->assignPrevNext(ax, prev);

	Face* abx = new Face{ ab };
	Face* bcx = new Face{ next };
	Face* cax = new Face{ prev };

	faceVect.insert(faceVect.end(), { abx,bcx,cax });

	ab->assingFace(abx);
	xa->assingFace(abx);
	bx->assingFace(abx);

	next->assingFace(bcx);
	xb->assingFace(bcx);
	cx->assingFace(bcx);

	prev->assingFace(cax);
	xc->assingFace(cax);
	ax->assingFace(cax);


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
	HalfEdge* ab = edgeToSwap;
	HalfEdge* ba = ab->getTwin();

	HalfEdge* ac = ba->getNext();
	HalfEdge* ca = ac->getTwin();

	/*HalfEdge* cb = ba->getPrev();
	HalfEdge* bc = cb->getTwin();*/

	HalfEdge* bd = ab->getNext();
	HalfEdge* db = bd->getTwin();

	/*HalfEdge* da = ab->getPrev();
	HalfEdge* ad = da->getTwin();*/

	ac->changeorigin(db->getOrigin());
	bd->changeorigin(ca->getOrigin());

	ac->assignTwin(bd);
	bd->assignTwin(ac);

	ab->assignTwin(ca);
	ca->assignTwin(ab);
	ba->assignTwin(db);
	db->assignTwin(ba);
}

//void Canvas::populateCanvas() {
//	for (auto& elem : boundaryVect) {
//		insertInFace(elem);
//	}
//}

