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

	friend void connectTriangle(HalfEdge* first, HalfEdge* second, HalfEdge* third);
	friend void connectTwins(HalfEdge* first, HalfEdge* second);

	void changeOrigin(Vertex* vertex);
	void assignTwin(HalfEdge* twin);
	void assingTriangle(Face* triangle);

};

class Face {					// any halfedge to the left of the face
	HalfEdge* Boundary;

public:

	Face(HalfEdge* boundary);

	HalfEdge* getEdge();

	HalfEdge* edgeContainingPoint(Point point);

	std::tuple<Point, Point, Point> getVertices();

	std::tuple<HalfEdge*, HalfEdge*, HalfEdge*> getEdges();

	bool containsAtleastOne(std::tuple<Point, Point, Point> boudingPoints);
	
	std::optional<std::variant<Face*, HalfEdge*>> contains(Point point);
};

class Canvas {
	std::vector<Point> inputPoints;
	std::vector< Vertex*> vertexVect;
	std::vector< HalfEdge*> edgeVect;
	std::vector< Face*> triangleVect;

	// vkládání bodù
	struct InsertStruct {
		Canvas& parent;
		Point point{};
		
		InsertStruct(Canvas& canvas) : parent(canvas) {};

		void operator() (Face* triangle) { parent.insertInFace(triangle, point); };
		void operator() (HalfEdge* halfedge) { halfedge->isBoundary() ? parent.insertInBoundaryEdge(halfedge, point) : parent.insertInInnerEdge(halfedge, point); };
	} insertFunctor;

	void insertInFace(Face* face, Point& point);
	void insertInInnerEdge(HalfEdge* halfedge, Point& point);
	void insertInBoundaryEdge(HalfEdge* halfedge, Point& point);

	// odstraòování
	void eraseEdge(HalfEdge* halfedge);
	bool cleanTriangle(Face* triangle, std::tuple<Point, Point, Point> boundingPoints);
	bool cleanVertex(Vertex* vertex, std::tuple<Point, Point, Point> boundingPoints);

	// tvorba CDT
	void makeEnclosingTriangle(std::tuple<Point, Point, Point> boundingPoints);
	void populateCanvas();
	void removeEnclosingTriangle(std::tuple<Point, Point, Point> boundingPoints);
	//void removeAdditionalEdges();
	
	// pomocné funkce
	void flipEdge(HalfEdge* edgeToSwap);
	void swapNecessary(HalfEdge* swapEdge);
	//void reconnectVertex(Vertex* vertex);
	//void removeTriangleVertex(Vertex* vertex);

	//HalfEdge* findStartingEdge();
	//HalfEdge* findHalfEdge(Vertex* origin, Point midpoint);


public:
	Canvas(std::vector<Point> boundary);
	void printByEdges();
	void insertPoint(Point point);
};