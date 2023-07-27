#pragma once

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

	void printCoords();

	void assignEdge(HalfEdge* halfedge);

	HalfEdge* getLeftmostEdge();

	HalfEdge* getEdge();
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
	void printVertices();

	bool contains(Vertex vertex);
};

class Canvas {
	std::vector<Point> boundaryVect;
	std::vector< Vertex*> verticesVect;
	std::vector< HalfEdge*> edgesVect;
	std::vector< Face*> faceVect;

public:
	Canvas(std::vector<Point> boundary);

	Face* findFace(Vertex x);

	std::tuple<Vertex*, Vertex*, Vertex*> pointsToVertices(std::tuple<Point, Point, Point> trianglePoints);

	std::tuple<HalfEdge*, HalfEdge*> makeTwins(Vertex* left, Vertex* right);

	void makeEnclosingTriangle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices);
	void swapNecessary(HalfEdge* swapEdge);

	void printFaces();
	void printFacesByEdges();

	void insertInFace(Point a);
	void deleteFace(Face* face);
	void deleteEdge(HalfEdge* halfedge);
	void deleteVertex(Vertex* vertex);

	void flipEdge(HalfEdge* edgeToSwap);
	void reconnectVertex(Vertex* vertex);

	void populateCanvas();

	void deleteTriangleVertex(Vertex* vertex);
	void removeEnclosingTrinagle(std::tuple<Vertex*, Vertex*, Vertex*> triangleVertices);
};