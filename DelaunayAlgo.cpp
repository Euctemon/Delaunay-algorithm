#include<tuple>
#include<iostream>
#include<algorithm>
#include<variant>
#include<optional>

#include "TrigComputations.h"
#include "DelaunayAlgo.h"

// magic
template<typename ... Ts>
struct Overload : Ts ... { using Ts::operator() ...; };
// end of magic


Vertex::Vertex(Point& point) {
	x = point.x;
	y = point.y;
	incident = nullptr;
}

Point Vertex::asPoint() { return Point{ x,y }; }

void Vertex::printCoords() { std::cout << x << '\t' << y << '\n'; }

void Vertex::assignEdge(HalfEdge* halfedge) { incident = halfedge; }

HalfEdge* Vertex::getLeftmostEdge() {
	HalfEdge* currentEdge = incident;
	while (!currentEdge->isBoundary()) {
		currentEdge = currentEdge->getPrev()->getTwin();
	}
	return currentEdge;
}

HalfEdge* Vertex::getEdge() { return incident; }

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

HalfEdge* HalfEdge::getTwin() { return Twin; }

Face* HalfEdge::getFace() { return Triangle; }

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
	else std::cout << "outer triangle";
}

void Canvas::insertPoint(Point point, bool includeEdges) {
	Face* face{ nullptr };
	HalfEdge* halfedge{ nullptr };

	auto f = [this, point](Face* face) {insertInFace(face, point); };
	auto g = [this, point, includeEdges](HalfEdge* halfedge) {if (includeEdges) insertInEdge(halfedge, point); };

	for (auto elem : faceVect) {
		if (auto foo = elem->contains(point)) {
			std::visit(Overload{ f,g }, foo.value());
			break;
		}
	}
}

std::optional<std::variant<Face*, HalfEdge*>> Face::contains(Point point) {
	auto&& [a, b, c] = this->getVertices();
	pointPos position = inTriangle(a.asPoint(), b.asPoint(), c.asPoint(), point);

	switch (position) {
	case INSIDE:
		return this;
	case BOUNDARY:
		return findEdgeToInsert(point);
	case OUTSIDE:
		return {};
	}
}

HalfEdge* Face::findEdgeToInsert(Point point) {
	HalfEdge* ab = this->getEdge();
	HalfEdge* bc = ab->getNext();
	HalfEdge* ca = ab->getPrev();

	if (orientedTriangle(ab->getOrigin()->asPoint(), ab->getTarget()->asPoint(), point) == 0) return ab;
	if (orientedTriangle(bc->getOrigin()->asPoint(), bc->getTarget()->asPoint(), point) == 0) return bc;
	else return ca;
}


std::tuple<Vertex*, Vertex*, Vertex*> Canvas::pointsToVertices(std::tuple<Point, Point, Point> trianglePoints) {
	Vertex* a = new Vertex{ std::get<0>(trianglePoints) };
	Vertex* b = new Vertex{ std::get<1>(trianglePoints) };
	Vertex* c = new Vertex{ std::get<2>(trianglePoints) };

	return std::make_tuple(a, b, c);
}

Canvas::Canvas(std::vector<Point> boundary) {
	boundaryVect = boundary;
	std::tuple<Vertex*, Vertex*, Vertex*> trianglePoints = pointsToVertices(boundingTrianglePoints(boundary));
	makeEnclosingTriangle(trianglePoints);
	populateCanvas();
	removeEnclosingTrinagle(trianglePoints);
	//removeAdditionalEdges();
}

std::tuple<HalfEdge*, HalfEdge*> Canvas::makeTwins(Vertex* left, Vertex* right) {
	HalfEdge* lr = new HalfEdge{ left };
	HalfEdge* rl = new HalfEdge{ right };

	lr->assignTwin(rl);
	rl->assignTwin(lr);

	return std::make_tuple(lr, rl);
}

void Canvas::makeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices) {
	Vertex* a = std::get<0>(triangleVertices);
	Vertex* b = std::get<1>(triangleVertices);
	Vertex* c = std::get<2>(triangleVertices);


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

	a->assignEdge(ab);
	b->assignEdge(bc);
	c->assignEdge(ca);

	this->verticesVect.assign({ a,b,c });
	this->edgesVect.assign({ ab,ba, ac, ca, bc, cb });
	this->faceVect.assign({ innerFace });
}

void Canvas::swapNecessary(HalfEdge* swapEdge) {
	if (swapEdge->getTwin()->isBoundary()) return;

	HalfEdge* left = swapEdge->getNext()->getTwin();
	HalfEdge* right = swapEdge->getPrev()->getTwin();

	Vertex* a = swapEdge->getOrigin();
	Vertex* b = swapEdge->getTarget();
	Vertex* p = swapEdge->getPrev()->getOrigin();
	Vertex* d = swapEdge->getTwin()->getNext()->getTarget();

	if (inCircle(a->asPoint(), b->asPoint(), p->asPoint(), d->asPoint())) {
		flipEdge(swapEdge);														// flipedge meni strukturu trojuhelniku, next uz neni validni
		swapNecessary(left->getTwin()->getPrev());
		swapNecessary(right->getTwin()->getNext());
	}
}

void Canvas::printFaces() {
	for (auto& elem : faceVect) {
		elem->printVertices();
		std::cout << '\n';
	}
}

void Canvas::insertInFace(Face* face, Point point) {
	Vertex* vertex = new Vertex{ point };

	HalfEdge* ab = face->getEdge();
	HalfEdge* next = ab->getNext();
	HalfEdge* prev = ab->getPrev();

	Vertex* a = ab->getOrigin();
	Vertex* b = next->getOrigin();
	Vertex* c = prev->getOrigin();

	auto&& [ax, xa] = makeTwins(a, vertex);
	auto&& [bx, xb] = makeTwins(b, vertex);
	auto&& [cx, xc] = makeTwins(c, vertex);

	vertex->assignEdge(xa);

	ab->assignPrevNext(xa, bx);
	bx->assignPrevNext(ab, xa);
	xa->assignPrevNext(bx, ab);

	next->assignPrevNext(xb, cx);
	cx->assignPrevNext(next, xb);
	xb->assignPrevNext(cx, next);


	prev->assignPrevNext(xc, ax);
	ax->assignPrevNext(prev, xc);
	xc->assignPrevNext(ax, prev);

	Face* abx = new Face{ ab };
	Face* bcx = new Face{ next };
	Face* cax = new Face{ prev };

	verticesVect.push_back(vertex);
	edgesVect.insert(edgesVect.end(), { ax,xa,bx,xb,cx,xc });
	faceVect.insert(faceVect.end(), { abx,bcx,cax });

	ab->assingFace(abx);
	bx->assingFace(abx);
	xa->assingFace(abx);

	next->assingFace(bcx);
	cx->assingFace(bcx);
	xb->assingFace(bcx);

	prev->assingFace(cax);
	ax->assingFace(cax);
	xc->assingFace(cax);

	deleteFace(face);
	swapNecessary(ab);
	swapNecessary(xc->getNext());
	swapNecessary(cx->getPrev());				// fliEdge uvnitr swap meni strukturu trojuhelniku, next pro ab uz neni validni
}

void Canvas::deleteFace(Face* face) {
	auto faceIt = std::find(faceVect.begin(), faceVect.end(), face);
	delete* faceIt;
	faceVect.erase(faceIt);
}

void Canvas::flipEdge(HalfEdge* edgeToSwap) {
	HalfEdge* ab = edgeToSwap;
	HalfEdge* ba = ab->getTwin();

	HalfEdge* ac = ba->getNext();
	HalfEdge* ca = ac->getTwin();

	HalfEdge* cb = ba->getPrev();
	HalfEdge* bc = cb->getTwin();

	HalfEdge* bd = ab->getNext();
	HalfEdge* db = bd->getTwin();

	HalfEdge* da = ab->getPrev();
	HalfEdge* ad = da->getTwin();

	ba->changeorigin(da->getOrigin());
	ab->changeorigin(ca->getOrigin());

	da->assignTwin(cb);
	cb->assignTwin(da);

	ab->assignTwin(bc);
	bc->assignTwin(ab);

	ba->assignTwin(ad);
	ad->assignTwin(ba);
}

void Canvas::populateCanvas() {
	for (auto& elem : boundaryVect) {
		insertPoint(elem, true);
	}
}

void Canvas::insertInEdge(HalfEdge* halfedge, Point point) {
	Vertex* vertex = new Vertex{ point };
	HalfEdge* ac = halfedge->getNext()->getTwin();
	HalfEdge* cd = halfedge->getTwin()->getPrev()->getTwin();

	auto&& [cx, xc] = makeTwins(cd->getOrigin(), vertex);

	HalfEdge* xd = new HalfEdge{ vertex };
	HalfEdge* ax = new HalfEdge{ ac->getOrigin() };

	HalfEdge* dc = new HalfEdge{ cd->getTarget() };
	HalfEdge* ca = new HalfEdge{ cd->getOrigin() };

	halfedge->getTwin()->changeorigin(vertex);
	halfedge->getNext()->changeorigin(vertex);

	ca->assignTwin(ac);
	ac->assignTwin(ca);

	cd->assignTwin(dc);
	dc->assignTwin(cd);

	ax->assignTwin(halfedge->getNext());
	halfedge->getNext()->assignTwin(ax);

	xd->assignTwin(halfedge->getTwin()->getPrev());
	halfedge->getTwin()->getPrev()->assignTwin(xd);

	ax->assignPrevNext(ca, xc);
	xc->assignPrevNext(ax, ca);
	ca->assignPrevNext(xc, ax);

	cx->assignPrevNext(dc, xd);
	xd->assignPrevNext(cx, dc);
	dc->assignPrevNext(xd, cx);

	Face* axc = new Face{ ax };
	Face* cxd = new Face{ cx };

	verticesVect.push_back(vertex);
	edgesVect.insert(edgesVect.end(), { cx,xc,xd,ax,dc,ca });
	faceVect.insert(faceVect.end(), { axc,cxd });

	swapNecessary(ax->getTwin()->getNext());
	swapNecessary(ax->getPrev());
	swapNecessary(xd->getTwin()->getPrev());
	swapNecessary(xd->getNext());
}

void Canvas::printFacesByEdges() {
	for (auto& elem : edgesVect) {
		if (!elem->isBoundary()) {
			elem->printOrigin();
			elem->getNext()->printOrigin();
			elem->getPrev()->printOrigin();
			std::cout << "\n ----- \n";
		}
	}
}

void Canvas::reconnectVertex(Vertex* vertex) {
	for (auto& halfedge : edgesVect) {
		if (halfedge->getOrigin() == vertex) {
			vertex->assignEdge(halfedge);
			break;
		}
		if (halfedge->getTarget() == vertex) {
			vertex->assignEdge(halfedge->getTwin());
			break;
		}
	}
}

void Canvas::deleteEdge(HalfEdge* halfedge) {
	HalfEdge* twin = halfedge->getTwin();

	auto edgeIt = std::find(edgesVect.begin(), edgesVect.end(), halfedge);
	delete* edgeIt;
	edgesVect.erase(edgeIt);

	auto twinIt = std::find(edgesVect.begin(), edgesVect.end(), twin);
	delete* twinIt;
	edgesVect.erase(twinIt);
}

void Canvas::deleteVertex(Vertex* vertex) {
	auto vertIt = std::find(verticesVect.begin(), verticesVect.end(), vertex);
	delete* vertIt;
	verticesVect.erase(vertIt);
}


void Canvas::removeTriangleVertex(Vertex* vertex) {
	reconnectVertex(vertex);
	vertex->assignEdge(vertex->getLeftmostEdge());

	HalfEdge* currentEdge = vertex->getEdge();
	HalfEdge* nextEdge = currentEdge->getTwin()->getNext();
	Vertex* vertexToConnect{};

	if (nextEdge == nullptr) {
		vertexToConnect = currentEdge->getTarget();
		deleteEdge(currentEdge);
		reconnectVertex(vertexToConnect);
	}
	else {
		HalfEdge* oppositeEdge = currentEdge->getTwin()->getPrev();
		Face* currentFace = nextEdge->getFace();

		while (!nextEdge->getTwin()->isBoundary()) {
			vertexToConnect = currentEdge->getTarget();
			deleteEdge(currentEdge);
			reconnectVertex(vertexToConnect);
			deleteFace(currentFace);

			oppositeEdge->assignPrevNext(nullptr, nullptr);
			oppositeEdge->assingFace(nullptr);



			currentEdge = nextEdge;
			nextEdge = currentEdge->getTwin()->getNext();
			oppositeEdge = currentEdge->getTwin()->getPrev();
			currentFace = nextEdge->getFace();
		}

		vertexToConnect = currentEdge->getTarget();
		deleteEdge(currentEdge);
		reconnectVertex(vertexToConnect);
		deleteEdge(nextEdge);
		deleteFace(currentFace);

		oppositeEdge->assignPrevNext(nullptr, nullptr);
		oppositeEdge->assingFace(nullptr);
	}

	deleteVertex(vertex);
}


bool Canvas::areNeighbours(Point first, Point second)
{
	auto firstIt = std::find(boundaryVect.begin(), boundaryVect.end(), first);
	if (std::next(firstIt) == boundaryVect.end()) return (boundaryVect.front() == second);
	else return (*std::next(firstIt) == second);
}

void Canvas::removeEnclosingTrinagle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices) {
	const auto& [a, b, c] = triangleVertices;
	removeTriangleVertex(a);
	removeTriangleVertex(c);
	removeTriangleVertex(b);
}

HalfEdge* Canvas::findStartingEdge() {
	Vertex* prev{ nullptr };
	Vertex* next{ nullptr };
	HalfEdge* halfedge{ nullptr };

	for (auto& elem : edgesVect) {
		prev = elem->getOrigin();
		next = elem->getTarget();
		if (elem->isBoundary() && areNeighbours(prev->asPoint(), next->asPoint())) {
			halfedge = elem;
			break;
		}
	}
	return halfedge;
}

void Canvas::removeAdditionalEdges() {
	Vertex* startingVertex{ nullptr };
	Vertex* currentVertex{ nullptr };
	HalfEdge* oppositeEdge{ nullptr };
	HalfEdge* currentEdge{ nullptr };
	Face* face{ nullptr };

	currentEdge = findStartingEdge();
	startingVertex = currentEdge->getOrigin();
	currentVertex = currentEdge->getTarget();


	while (currentVertex != startingVertex) {
		currentVertex->assignEdge(currentEdge->getTwin());

		while (!areNeighbours(currentVertex->asPoint(), currentVertex->getLeftmostEdge()->getTarget()->asPoint())) {
			oppositeEdge = currentVertex->getLeftmostEdge()->getTwin()->getPrev();
			deleteEdge(currentVertex->getLeftmostEdge());
			deleteFace(oppositeEdge->getFace());

			oppositeEdge->getPrev()->assingFace(nullptr);
			oppositeEdge->assingFace(nullptr);
		}

		currentEdge = currentVertex->getLeftmostEdge();
		currentEdge->assignPrevNext(nullptr, nullptr);
		currentVertex = currentVertex->getLeftmostEdge()->getTarget();
	}
	currentVertex->assignEdge(currentEdge->getTwin());
}