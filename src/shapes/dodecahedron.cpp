#include "shapes/dodecahedron.h"
#include <array>
#include <vector>
#include "shapes.h"
#include "shapes/fractal.h"
#include "shapes/icosahedron.h"

using namespace std;

namespace shapes {
	constexpr static auto points = []() constexpr {
		array<Point3D, 20> points = {};
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
		return points;
	}();

	constexpr static auto edges = []() constexpr {
		array<Edge, 30> edges = {};
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
		return edges;
	}();

	constexpr static auto faces = []() constexpr {
		array<Face, 36> faces = {};

		// FIXME Copied straight from cursus but we have different points so
		// it resembles an eldritch horror.
		// Deducing the points manually is currently very hard since we can't
		// see the edges between faces. When lighting based on normals is
		// implemented we can fix it more easily.
		struct { unsigned int a, b, c, d, e; } fives[12] = {
			// Top & bottom
			{  0,  1,  2,  3,  4 },
			{  9,  8,  7,  6,  5 },
			// Top ring
			{  0,  4, 19, 10, 15 },
			{  1,  0, 15, 11, 16 },
			{  2,  1, 16, 12, 17 },
			{  3,  2, 17, 13, 18 },
			{  4,  3, 18, 14, 19 },
			// Bottom ring
			{  9,  5, 10, 19, 14 },
			{  5,  6, 11, 15, 10 },
			{  6,  7, 12, 16, 11 },
			{  7,  8, 13, 17, 12 },
			{  8,  9, 14, 18, 13 },
		};

		for (unsigned int i = 0; i < 12; i++) {
			faces[i * 3 + 0] = { fives[i].a, fives[i].b, fives[i].c };
			faces[i * 3 + 1] = { fives[i].a, fives[i].c, fives[i].d };
			faces[i * 3 + 2] = { fives[i].a, fives[i].d, fives[i].e };
		}

		return faces;
	}();

	void dodecahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		platonic(conf, lines, points, edges);
	}

	TriangleFigure dodecahedron(const FigureConfiguration &conf) {
		return platonic(conf, points, faces);
	}

	void fractal_dodecahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector p(points.begin(), points.end());
		vector e(edges.begin(), edges.end());
		fractal(conf, p, e);
		platonic(conf, lines, p, e);
	}

	TriangleFigure fractal_dodecahedron(const FigureConfiguration &conf) {
		vector p(points.begin(), points.end());
		vector f(faces.begin(), faces.end());
		fractal(conf, p, f);
		return platonic(conf, p, f);
	}
}
