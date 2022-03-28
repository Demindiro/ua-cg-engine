#include "shapes/dodecahedron.h"
#include <vector>
#include "shapes.h"
#include "shapes/fractal.h"
#include "shapes/icosahedron.h"

using namespace std;

namespace shapes {
	static void dodecahedron(Point3D points[20]) {
		Point3D ico[12];
		icosahedron(ico);
		for (unsigned int i = 0; i < 5; i++) {
			unsigned int j = (i + 1) % 5;
			// Top & bottom "hat"
			points[0 + i] = Point3D::center({ ico[0], ico[2 + i], ico[2 + j] });
			points[5 + i] = Point3D::center({ ico[1], ico[7 + i], ico[7 + j] });
			// Ring
			points[10 + i] = Point3D::center({ ico[2 + i], ico[7 + i], ico[7 + j] });
			points[15 + i] = Point3D::center({ ico[2 + i], ico[2 + j], ico[7 + j] });
		}
	}

	static void dodecahedron(Edge edges[30]) {
		for (unsigned int i = 0; i < 5; i++) {
			unsigned int j = (i + 1) % 5;
			// Top & bottom ring
			edges[0 + i] = { 0 + i, 0 + j };
			edges[5 + i] = { 5 + i, 5 + j };
			// Top & bottom "surface" ring
			edges[10 + i] = { 0 + i, 15 + i };
			edges[15 + i] = { 5 + i, 10 + i };
			// Middle ring
			edges[20 + i] = { 10 + i, 15 + i };
			edges[25 + i] = { 10 + j, 15 + i };
		}
	}

	static void dodecahedron(Face faces[36]) {
		// FIXME Copied straight from cursus but we have different points so
		// it resembles an eldritch horror.
		// Deducing the points manually is currently very hard since we can't
		// see the edges between faces. When lighting based on normals is
		// implemented we can fix it more easily.
		struct { unsigned int a, b, c, d, e; } fives[12] = {
			{  1,  2,  3,  4,  5 },
			{  1,  6,  7,  8,  2 },
			{  2,  8,  9, 10,  3 },
			{  3, 10, 11, 12,  4 },
			{  4, 12, 13, 14,  5 },
			{  5, 14, 15,  6,  1 },
			{ 20, 19, 18, 17, 16 },
			{ 20, 15, 14, 13, 19 },
			{ 19, 13, 12, 11, 18 },
			{ 18, 11, 10,  9, 17 },
			{ 17,  9,  8,  7, 16 },
			{ 16,  7,  6, 15, 20 },
		};

		for (unsigned int i = 0; i < 12; i++) {
			faces[i * 3 + 0] = { fives[i].a - 1, fives[i].b - 1, fives[i].c - 1 };
			faces[i * 3 + 1] = { fives[i].a - 1, fives[i].c - 1, fives[i].d - 1 };
			faces[i * 3 + 2] = { fives[i].a - 1, fives[i].d - 1, fives[i].e - 1 };
		}
	}

	void dodecahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		Point3D points[20];
		Edge edges[30];
		dodecahedron(points);
		dodecahedron(edges);
		platonic(conf, lines, points, 20, edges, 30);
	}

	TriangleFigure dodecahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(20);
		vector<Face> faces(36);
		dodecahedron(points.data());
		dodecahedron(faces.data());
		return platonic(conf, points, faces);
	}

	void fractal_dodecahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points(20);
		vector<Edge> edges(30);
		dodecahedron(points.data());
		dodecahedron(edges.data());
		fractal(conf, points, edges);
		platonic(conf, lines, points, edges);
	}

	TriangleFigure fractal_dodecahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(20);
		vector<Face> faces(36);
		dodecahedron(points.data());
		dodecahedron(faces.data());
		fractal(conf, points, faces);
		return platonic(conf, points, faces);
	}
}
