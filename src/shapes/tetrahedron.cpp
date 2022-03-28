#include "shapes/tetrahedron.h"
#include "shapes.h"
#include "shapes/fractal.h"

using namespace std;

namespace shapes {
	static void tetrahedron(Point3D points[4]) {
		points[0] = {  1, -1, -1 };
		points[1] = { -1,  1, -1 };
		points[2] = {  1,  1,  1 };
		points[3] = { -1, -1,  1 };
	}

	static void tetrahedron(Edge edges[6]) {
		edges[0] = { 0, 1 };
		edges[1] = { 0, 2 };
		edges[2] = { 0, 3 };
		edges[3] = { 1, 2 };
		edges[4] = { 1, 3 };
		edges[5] = { 2, 3 };
	}

	static void tetrahedron(Face faces[4]) {
		faces[0] = { 0, 1, 2 };
		faces[1] = { 0, 1, 3 };
		faces[2] = { 0, 2, 3 };
		faces[3] = { 2, 1, 3 };
	}

	void tetrahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		Point3D points[4];
		Edge edges[6];
		tetrahedron(points);
		tetrahedron(edges);
		platonic(conf, lines, points, 4, edges, 6);
	}

	TriangleFigure tetrahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(4);
		vector<Face> faces(4);
		tetrahedron(points.data());
		tetrahedron(faces.data());
		return platonic(conf, points, faces);
	}

	void fractal_tetrahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points(4);
		vector<Edge> edges(6);
		tetrahedron(points.data());
		tetrahedron(edges.data());
		fractal(conf, points, edges);
		platonic(conf, lines, points, edges);
	}

	TriangleFigure fractal_tetrahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(4);
		vector<Face> faces(4);
		tetrahedron(points.data());
		tetrahedron(faces.data());
		fractal(conf, points, faces);
		return platonic(conf, points, faces);
	}
}
