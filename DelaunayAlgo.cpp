#include<tuple>
#include<iostream>
#include<algorithm>

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

Face* HalfEdge::getFace() { return Triangle;}

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

void Canvas::insertInFace(Point point) {
	Vertex* vertex = new Vertex{ point };

	Face* oldTriangle = findFace(*vertex);
	HalfEdge* ab = oldTriangle->getEdge();
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

	deleteFace(oldTriangle);
	swapNecessary(ab);
	swapNecessary(xc->getNext());
	swapNecessary(cx->getPrev());				// fliEdge uvnitr swap meni strukturu trojuhelniku, next pro ab uz neni validni
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
		insertInFace(elem);
	}
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

	for (auto elemIt = edgesVect.begin(); elemIt < edgesVect.end(); elemIt++) {
		if (*elemIt == halfedge) {
			delete* elemIt;
			edgesVect.erase(elemIt);
			break;
		}
	}
	for (auto elemIt = edgesVect.begin(); elemIt < edgesVect.end(); elemIt++) {
		if (*elemIt == twin) {
			delete* elemIt;
			edgesVect.erase(elemIt);
			break;
		}
	}
}

void Canvas::deleteVertex(Vertex* vertex) {
	for (auto elemIt = verticesVect.begin(); elemIt < verticesVect.end(); elemIt++) {
		if (*elemIt == vertex) {
			delete* elemIt;
			verticesVect.erase(elemIt);
			break;
		}
	}
}


void Canvas::deleteTriangleVertex(Vertex* vertex) {
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

void Canvas::removeEnclosingTrinagle(std::tuple<Vertex*,Vertex*,Vertex*> triangleVertices) {
	const auto& [a, b, c] = triangleVertices;
	deleteTriangleVertex(a);
	deleteTriangleVertex(c);
	deleteTriangleVertex(b);
}

