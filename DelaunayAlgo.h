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
	
	HalfEdge* leftBoundaryEdge(HalfEdge* halfedge);
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

	std::tuple<HalfEdge*, HalfEdge*> makeTwins(Vertex* left, Vertex* right);

	void makeEnclosingTriangle(std::tuple<Point, Point, Point> trianglePoints);
	void swapNecessary(HalfEdge* swapEdge);

	void printFaces();
	void insertInFace(Point a);
	void deleteFace(Face* face);

	void flipEdge(HalfEdge* edgeToSwap);

	void populateCanvas();
};