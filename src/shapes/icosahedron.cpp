#include "shapes/icosahedron.h"
#include <vector>
#include "lines.h"
#include "shapes.h"
#include "shapes/fractal.h"

using namespace std;

namespace shapes {
	void icosahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		Point3D points[12];
		Edge edges[30];
		icosahedron(points);
		icosahedron(edges);
		platonic(conf, lines, points, 12, edges, 30);
	}

	TriangleFigure icosahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(12);
		vector<Face> faces(20);
		icosahedron(points.data());
		icosahedron(faces.data());
		return platonic(conf, points, faces);
	}

	void fractal_icosahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points(12);
		vector<Edge> edges(30);
		icosahedron(points.data());
		icosahedron(edges.data());
		fractal(conf, points, edges);
		platonic(conf, lines, points, edges);
	}

	TriangleFigure fractal_icosahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(12);
		vector<Face> faces(20);
		icosahedron(points.data());
		icosahedron(faces.data());
		fractal(conf, points, faces);
		return platonic(conf, points, faces);
	}
}
