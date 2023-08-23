#include<tuple>
#include<iostream>
#include<algorithm>
#include<optional>
#include<functional>

#include "TrigComputations.h"
#include "DelaunayAlgo.h"

// magic
template<typename ... Ts>
struct Overload : Ts ... { using Ts::operator() ...; };
// end of magic


// VRCHOLY

Vertex::Vertex(Point& point) {
	x = point.x;
	y = point.y;
	Incident = nullptr;
}

Point Vertex::asPoint() { return Point{ x,y }; }

HalfEdge* Vertex::getLeftmostEdge() {
	HalfEdge* currentEdge = Incident;
	while (!currentEdge->isBoundary()) {
		currentEdge = currentEdge->getPrev()->getTwin();
	}
	return currentEdge;
}

HalfEdge* Vertex::getEdge() { return Incident; }

void Vertex::assignEdge(HalfEdge* halfedge) { Incident = halfedge; }


// HRANY

HalfEdge::HalfEdge(Vertex* origin) {
	Origin = origin;
	Next = nullptr;
	Prev = nullptr;
	Triangle = nullptr;
	Twin = nullptr;
}

Vertex* HalfEdge::getOrigin() { return Origin; }

Vertex* HalfEdge::getTarget() { return Twin->getOrigin(); }

HalfEdge* HalfEdge::getNext() { return Next; }

HalfEdge* HalfEdge::getPrev() { return Prev; }

HalfEdge* HalfEdge::getTwin() { return Twin; }

Face* HalfEdge::getFace() { return Triangle; }

bool HalfEdge::isBoundary() { return (Triangle == nullptr); }

bool HalfEdge::shouldSplit() {
	Point origin = Origin->asPoint();
	Point target = getTarget()->asPoint();
	Point pointToLeft = Prev->getOrigin()->asPoint();

	return isNearHalfEdge(origin, target, pointToLeft);
}

void HalfEdge::changeorigin(Vertex* vertex) { Origin = vertex; };

void HalfEdge::assignPrevNext(HalfEdge* prev, HalfEdge* next) {
	Prev = prev;
	Next = next;
}

void HalfEdge::assignTwin(HalfEdge* halfEdge) { Twin = halfEdge; }

void HalfEdge::assingFace(Face* triangle) { Triangle = triangle; }


// TROJÚHELNÍKY

Face::Face(HalfEdge* boundary) {
	Boundary = boundary;
	if (boundary) {
		boundary->assingFace(this);
		boundary->getNext()->assingFace(this);
		boundary->getPrev()->assingFace(this);
	}
}

HalfEdge* Face::getEdge() { return Boundary; }

HalfEdge* Face::findEdgeToInsert(Point point) {
	HalfEdge* ab = getEdge();
	HalfEdge* bc = ab->getNext();
	HalfEdge* ca = ab->getPrev();

	if (orientedTriangle(ab->getOrigin()->asPoint(), ab->getTarget()->asPoint(), point) == 0) return ab;
	if (orientedTriangle(bc->getOrigin()->asPoint(), bc->getTarget()->asPoint(), point) == 0) return bc;
	else return ca;
}

std::tuple<Vertex, Vertex, Vertex> Face::getVertices() {
	Vertex* a = Boundary->getOrigin();
	Vertex* b = Boundary->getNext()->getOrigin();
	Vertex* c = Boundary->getPrev()->getOrigin();
	return std::make_tuple(*a, *b, *c);
}

std::optional<std::variant<Face*, HalfEdge*>> Face::contains(Point point) {
	auto&& [a, b, c] = getVertices();
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

// PLÁTNO

void Canvas::deleteFace(Face* face) {
	auto faceIt = std::find(triangleVect.begin(), triangleVect.end(), face);
	delete* faceIt;
	triangleVect.erase(faceIt);
}

void Canvas::deleteEdge(HalfEdge* halfedge) {
	HalfEdge* twin = halfedge->getTwin();

	auto edgeIt = std::find(edgeVect.begin(), edgeVect.end(), halfedge);
	delete* edgeIt;
	edgeVect.erase(edgeIt);

	auto twinIt = std::find(edgeVect.begin(), edgeVect.end(), twin);
	delete* twinIt;
	edgeVect.erase(twinIt);
}

void Canvas::deleteVertex(Vertex* vertex) {
	auto vertIt = std::find(vertexVect.begin(), vertexVect.end(), vertex);
	delete* vertIt;
	vertexVect.erase(vertIt);
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

	vertexVect.push_back(vertex);
	edgeVect.insert(edgeVect.end(), { ax,xa,bx,xb,cx,xc });
	triangleVect.insert(triangleVect.end(), { abx,bcx,cax });

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

void Canvas::insertInEdge(HalfEdge* halfedge, Point point) {
	if (halfedge->isBoundary() || halfedge->getTwin()->isBoundary()) insertInBoundaryEdge(halfedge, point);
	else insertInInnerEdge(halfedge, point);
}

void Canvas::insertInInnerEdge(HalfEdge* halfedge, Point point) {
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

	vertexVect.push_back(vertex);
	edgeVect.insert(edgeVect.end(), { cx,xc,xd,ax,dc,ca });
	triangleVect.insert(triangleVect.end(), { axc,cxd });

	swapNecessary(ax->getTwin()->getNext());
	swapNecessary(ax->getPrev());
	swapNecessary(xd->getTwin()->getPrev());
	swapNecessary(xd->getNext());
}

void Canvas::insertInBoundaryEdge(HalfEdge* halfedge, Point point) {
	Vertex* vertex = new Vertex{ point };

	HalfEdge* innerEdge = (halfedge->isBoundary() ? halfedge->getTwin() : halfedge);
	HalfEdge* ba = innerEdge->getNext()->getTwin();
	HalfEdge* cb = innerEdge->getPrev()->getTwin();

	auto&& [cx, xc] = makeTwins(cb->getOrigin(), vertex);

	HalfEdge* xb = new HalfEdge{ vertex };
	HalfEdge* bc = new HalfEdge{ ba->getOrigin() };

	innerEdge->changeorigin(vertex);

	xb->assignTwin(innerEdge->getPrev());
	innerEdge->getPrev()->assignTwin(xb);

	bc->assignTwin(cb);
	cb->assignTwin(bc);

	xb->assignPrevNext(cx, bc);
	bc->assignPrevNext(xb, cx);
	cx->assignPrevNext(bc, xb);

	xc->assignPrevNext(nullptr, nullptr);

	Face* xbc = new Face{ xb };

	vertexVect.push_back(vertex);
	edgeVect.insert(edgeVect.end(), { cx,xc,xb,bc });
	triangleVect.push_back(xbc);

	swapNecessary(innerEdge);
	swapNecessary(ba->getTwin());
	swapNecessary(cb->getTwin());
}

std::tuple<Vertex*, Vertex*, Vertex*> Canvas::pointsToVertices(std::tuple<Point, Point, Point> trianglePoints) {
	auto&& [a, b, c] = trianglePoints;
	Vertex* A = new Vertex{ a };
	Vertex* B = new Vertex{ b };
	Vertex* C = new Vertex{ c };

	return std::make_tuple(A, B, C);
}

void Canvas::makeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices) {
	auto&& [a, b, c] = triangleVertices;

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

	vertexVect.assign({ a,b,c });
	edgeVect.assign({ ab,ba, ac, ca, bc, cb });
	triangleVect.assign({ innerFace });
}

void Canvas::populateCanvas() {
	for (auto& point : boundaryVect) {
		insertPoint(point);
	}
}

void Canvas::removeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices) {
	const auto& [a, b, c] = triangleVertices;
	removeTriangleVertex(a);
	removeTriangleVertex(c);
	removeTriangleVertex(b);
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

void Canvas::swapNecessary(HalfEdge* swapEdge) {
	if (swapEdge->isBoundary() || swapEdge->getTwin()->isBoundary()) return;

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

void Canvas::reconnectVertex(Vertex* vertex) {
	for (auto& halfedge : edgeVect) {
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

void Canvas::splitSide(Vertex* origin, Vertex* target) {
	Point midpoint = getMidpoint(origin->asPoint(), target->asPoint());
	insertPoint(midpoint);

	HalfEdge* left = findHalfEdge(origin, midpoint);
	HalfEdge* right = findHalfEdge(target, midpoint);
	HalfEdge* current;

	if (left->isBoundary()) {
		current = left->getTwin();
		if (current->shouldSplit()) splitSide(current->getOrigin(), current->getTarget());
	}
	else if (left->getTwin()->isBoundary()) {
		current = left;
		if (current->shouldSplit()) splitSide(current->getOrigin(), current->getTarget());
	}

	if (right->isBoundary()) {
		current = right->getTwin();
		if (current->shouldSplit()) splitSide(current->getOrigin(), current->getTarget());
	}
	else if (right->getTwin()->isBoundary()) {
		current = right;
		if (current->shouldSplit()) splitSide(current->getOrigin(), current->getTarget());
	}
}

bool Canvas::areNeighbours(Point first, Point second)
{
	auto firstIt = std::find(boundaryVect.begin(), boundaryVect.end(), first);
	if (std::next(firstIt) == boundaryVect.end()) return (boundaryVect.front() == second);
	else return (*std::next(firstIt) == second);
}

bool Canvas::sameVertices(HalfEdge* halfedge, std::tuple<Vertex*, Vertex*> vertices)
{
	auto&& [origin, target] = vertices;
	Vertex* edgeOrigin = halfedge->getOrigin();
	Vertex* edgeTarget = halfedge->getTarget();

	bool areSame = (edgeOrigin == origin && edgeTarget == target) || (edgeOrigin == target && edgeTarget == origin);

	return areSame;
}

HalfEdge* Canvas::findStartingEdge() {
	Vertex* prev{ nullptr };
	Vertex* next{ nullptr };
	HalfEdge* startingEdge{ nullptr };

	for (auto& halfedge : edgeVect) {
		prev = halfedge->getOrigin();
		next = halfedge->getTarget();
		if (halfedge->isBoundary() && areNeighbours(prev->asPoint(), next->asPoint())) {
			startingEdge = halfedge;
			break;
		}
	}
	return startingEdge;
}

HalfEdge* Canvas::findHalfEdge(Vertex* origin, Point midpoint) {
	auto compareVertices = [&](HalfEdge* halfedge) {return origin == halfedge->getOrigin() && midpoint == halfedge->getTarget()->asPoint(); };
	auto halfedgeIt = std::find_if(begin(edgeVect), end(edgeVect), compareVertices);
	return (*halfedgeIt);
}

std::tuple<HalfEdge*, HalfEdge*> Canvas::makeTwins(Vertex* left, Vertex* right) {
	HalfEdge* lr = new HalfEdge{ left };
	HalfEdge* rl = new HalfEdge{ right };

	lr->assignTwin(rl);
	rl->assignTwin(lr);

	return std::make_tuple(lr, rl);
}

std::vector<std::tuple<Vertex*,Vertex*>> Canvas::getBadSides() {
	std::vector<std::tuple<Vertex*, Vertex*>> badEdges;
	Vertex* origin{};
	Vertex* target{};
	Point point{};
	HalfEdge* current{};

	for (auto& halfedge : edgeVect) {
		if (halfedge->isBoundary()) {
			current = halfedge->getTwin();
			origin = current->getOrigin();
			target = current->getTarget();
			point = current->getPrev()->getOrigin()->asPoint();
			if (isNearHalfEdge(origin->asPoint(), target->asPoint(), point)) {
				badEdges.push_back(std::make_tuple(origin,target));
			}
		}
	}
	return badEdges;
}

std::optional<Face*> Canvas::getBadTriangle()
{
	for (auto& triangle : triangleVect) {
		auto&& [a, b, c] = triangle->getVertices();
		if (hasBadAngle(a.asPoint(), b.asPoint(), c.asPoint()) || hasBadArea(a.asPoint(), b.asPoint(), c.asPoint())) return triangle;
	}
	return {};
}

std::optional<std::vector<HalfEdge*>> Canvas::getEncroachedEdges(Point circumcenter)
{
	Point origin{};
	Point target{};
	std::vector<HalfEdge*> edges{};
	HalfEdge* current;

	for (auto& halfedge : edgeVect) {
		if (halfedge->isBoundary()) {
			current = halfedge->getTwin();
			origin = current->getOrigin()->asPoint();
			target = current->getTarget()->asPoint();
			if (isNearHalfEdge(origin, target, circumcenter)) edges.push_back(halfedge);
		}
	}
	auto maybeEdges = edges.empty() ? std::nullopt : std::make_optional(edges);
	return maybeEdges;
}

Canvas::Canvas(std::vector<Point> boundary) {
	boundaryVect = boundary;
	std::tuple<Vertex*, Vertex*, Vertex*> trianglePoints = pointsToVertices(boundingTrianglePoints(boundary));
	makeEnclosingTriangle(trianglePoints);
	populateCanvas();
	removeEnclosingTriangle(trianglePoints);
	removeAdditionalEdges();
}

std::vector<HalfEdge*> Canvas::getEdges() {
	return edgeVect;
}

void Canvas::insertPoint(Point point) {
	Face* face{ nullptr };
	HalfEdge* halfedge{ nullptr };

	auto f = [this, point](Face* face) {insertInFace(face, point); };
	auto g = [this, point](HalfEdge* halfedge) {insertInEdge(halfedge, point); };

	for (auto triangle : triangleVect) {
		if (auto maybeContains = triangle->contains(point)) {
			std::visit(Overload{ f,g }, maybeContains.value());
			break;
		}
	}
}

void Canvas::Ruppert() {
	std::vector<std::tuple<Vertex*,Vertex*>> badSides = getBadSides();
	std::optional<Face*> badTriangle = getBadTriangle();

	bool insertedTriangle{false};

	int i{ 0 };
	while (i<20) {
		i++;
		for (auto& [origin, target] : badSides) {
			splitSide(origin, target);
		}
		badSides.clear();

		badTriangle = getBadTriangle();
		if (badTriangle.has_value()) {
			auto&& [a, b, c] = badTriangle.value()->getVertices();
			Point circumcenter = getCircumcenter(a.asPoint(), b.asPoint(), c.asPoint());
			auto maybeEdges = getEncroachedEdges(circumcenter);
			auto edgeIncluded = [&](HalfEdge* halfedge) {return std::find_if(begin(badSides), end(badSides), [&](std::tuple<Vertex*, Vertex*> tupleVer) {return sameVertices(halfedge, tupleVer); }) != badSides.end(); };
			
			if (maybeEdges.has_value()) {
				for (auto& side : maybeEdges.value()) {
					if (!edgeIncluded(side)) badSides.push_back(std::make_tuple(side->getOrigin(), side->getTarget()));
				}
			}
			else {
				insertPoint(circumcenter);
				insertedTriangle = true;
			}
		}

		if (badSides.empty() && !insertedTriangle) break;
		insertedTriangle = false;
	}
}
