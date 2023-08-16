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
	HalfEdge* incident;

public:
	Vertex(Point& point);

	Point asPoint();

	HalfEdge* getEdge();

	HalfEdge* getLeftmostEdge();

	void printCoords();

	void assignEdge(HalfEdge* halfedge);

	// zbavit se tohoto
	double getX();

	double getY();
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

	void printOrigin();
	void changeorigin(Vertex* vertex);

	void assignPrevNext(HalfEdge* prev, HalfEdge* next);
	void assignTwin(HalfEdge* twin);
	void assingFace(Face* triangle);
};


class Face {					// any halfedge to the left of the face
	HalfEdge* boundary;

public:

	Face(HalfEdge* boundary);

	HalfEdge* getEdge();

	std::tuple<Vertex, Vertex, Vertex> getVertices();

	HalfEdge* findEdgeToInsert(Point point);

	void printVertices();

	std::optional<std::variant<Face*, HalfEdge*>> contains(Point point);
};

class Canvas {
	std::vector<Point> boundaryVect;
	std::vector< Vertex*> verticesVect;
	std::vector< HalfEdge*> edgesVect;
	std::vector< Face*> faceVect;

public:
	Canvas(std::vector<Point> boundary);

	std::tuple<Vertex*, Vertex*, Vertex*> pointsToVertices(std::tuple<Point, Point, Point> trianglePoints);

	std::tuple<HalfEdge*, HalfEdge*> makeTwins(Vertex* left, Vertex* right);

	void makeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices);
	void swapNecessary(HalfEdge* swapEdge);

	void printFaces();
	void printFacesByEdges();

	void insertPoint(Point point, bool includeEdges);
	void insertInFace(Face* face, Point point);
	void insertInEdge(HalfEdge* halfedge, Point point);

	void insertInInnerEdge(HalfEdge* halfedge, Point point);
	void insertInBoundaryEdge(HalfEdge* halfedge, Point point);


	void deleteFace(Face* face);
	void deleteEdge(HalfEdge* halfedge);
	void deleteVertex(Vertex* vertex);

	void flipEdge(HalfEdge* edgeToSwap);
	void reconnectVertex(Vertex* vertex);

	void populateCanvas();

	void removeTriangleVertex(Vertex* vertex);
	void removeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices);

	bool areNeighbours(Point first, Point second);

	HalfEdge* findStartingEdge();

	void removeAdditionalEdges();

	void createMesh();

	void writeMeshCVS();

	void refineBoundary(float spacing);
};