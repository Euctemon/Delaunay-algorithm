#pragma once

#include<optional>
#include<variant>
#include<vector>
#include "TrigComputations.h"

class Face;
class HalfEdge;

class Vertex {					// any HalfEdge coming from that vertex
	double x;
	double y;
	HalfEdge* Incident;

public:
	Vertex(Point& point);

	Point asPoint();

	HalfEdge* getEdge();

	HalfEdge* getLeftmostEdge();

	void assignEdge(HalfEdge* halfedge);
};

class HalfEdge {
	Vertex* Origin;
	HalfEdge* Twin;
	HalfEdge* Prev;
	HalfEdge* Next;
	Face* Triangle;

public:
	HalfEdge(Vertex* origin);

	Vertex* getOrigin();
	Vertex* getTarget();

	HalfEdge* getNext();
	HalfEdge* getPrev();
	HalfEdge* getTwin();

	Face* getFace();

	bool isBoundary();
	bool shouldSplit();

	void changeorigin(Vertex* vertex);
	void assignPrevNext(HalfEdge* prev, HalfEdge* next);
	void assignTwin(HalfEdge* twin);
	void assingFace(Face* triangle);

};

class Face {					// any halfedge to the left of the face
	HalfEdge* Boundary;

public:

	Face(HalfEdge* boundary);

	HalfEdge* getEdge();

	HalfEdge* findEdgeToInsert(Point point);

	std::tuple<Vertex, Vertex, Vertex> getVertices();
	
	std::optional<std::variant<Face*, HalfEdge*>> contains(Point point);
};

class Canvas {
	std::vector<Point> boundaryVect;
	std::vector< Vertex*> vertexVect;
	std::vector< HalfEdge*> edgeVect;
	std::vector< Face*> triangleVect;

	// odstraòování
	void deleteFace(Face* face);
	void deleteEdge(HalfEdge* halfedge);
	void deleteVertex(Vertex* vertex);

	// vkládání
	void insertInFace(Face* face, Point point);
	void insertInEdge(HalfEdge* halfedge, Point point);
	void insertInInnerEdge(HalfEdge* halfedge, Point point);
	void insertInBoundaryEdge(HalfEdge* halfedge, Point point);

	// tvorba CDT
	std::tuple<Vertex*, Vertex*, Vertex*> pointsToVertices(std::tuple<Point, Point, Point> trianglePoints);

	void makeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices);
	void populateCanvas();
	void removeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices);
	void removeAdditionalEdges();
	
	// pomocné funkce
	void flipEdge(HalfEdge* edgeToSwap);
	void swapNecessary(HalfEdge* swapEdge);
	void reconnectVertex(Vertex* vertex);
	void removeTriangleVertex(Vertex* vertex);
	void splitSide(Vertex* origin, Vertex* target);

	bool areNeighbours(Point first, Point second);
	bool sameVertices(HalfEdge* halfedge, std::tuple<Vertex*, Vertex*> vertices);

	HalfEdge* findStartingEdge();
	HalfEdge* findHalfEdge(Vertex* origin, Point midpoint);

	std::tuple<HalfEdge*, HalfEdge*> makeTwins(Vertex* left, Vertex* right);

	std::vector<std::tuple<Vertex*, Vertex*>> getBadSides();
	
	std::optional<Face*> getBadTriangle();

	std::optional<std::vector<HalfEdge*>> getEncroachedEdges(Point circumcenter);

public:
	Canvas(std::vector<Point> boundary);

	std::vector<HalfEdge*> getEdges();

	void insertPoint(Point point);
	void Ruppert();
};