#include "shapes/octahedron.h"
#include <vector>
#include "shapes.h"
#include "shapes/fractal.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

	static void octahedron(Point3D points[6]) {
		points[0] = Point3D( 1,  0,  0);
		points[1] = Point3D(-1,  0,  0);
		points[2] = Point3D( 0,  1,  0);
		points[3] = Point3D( 0,  0,  1);
		points[4] = Point3D( 0, -1,  0);
		points[5] = Point3D( 0,  0, -1);
	}

	static void octahedron(Edge edges[12]) {
		// Top
		edges[0] = { 0, 2 };
		edges[1] = { 0, 3 };
		edges[2] = { 0, 4 };
		edges[3] = { 0, 5 };
		// Bottom
		edges[4] = { 1, 2 };
		edges[5] = { 1, 3 };
		edges[6] = { 1, 4 };
		edges[7] = { 1, 5 };
		// Ring
		edges[ 8] = { 2, 3 };
		edges[ 9] = { 3, 4 };
		edges[10] = { 4, 5 };
		edges[11] = { 5, 2 };
	}

	static void octahedron(Face faces[8]) {
		// Top
		faces[0] = { 0, 2, 3 };
		faces[1] = { 0, 3, 4 };
		faces[2] = { 0, 4, 5 };
		faces[3] = { 0, 5, 2 };
		// Bottom
		faces[4] = { 2, 1, 3 };
		faces[5] = { 3, 1, 4 };
		faces[6] = { 4, 1, 5 };
		faces[7] = { 5, 1, 2 };
	}

	void octahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		Point3D points[6];
		Edge edges[12];
		octahedron(points);
		octahedron(edges);
		platonic(conf, lines, points, 6, edges, 12);
	}

	TriangleFigure octahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(6);
		vector<Face> faces(8);
		octahedron(points.data());
		octahedron(faces.data());
		return platonic(conf, points, faces);
	}

	void fractal_octahedron(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points(6);
		vector<Edge> edges(12);
		octahedron(points.data());
		octahedron(edges.data());
		fractal(conf, points, edges);
		platonic(conf, lines, points, edges);
	}

	TriangleFigure fractal_octahedron(const FigureConfiguration &conf) {
		vector<Point3D> points(6);
		vector<Face> faces(8);
		octahedron(points.data());
		octahedron(faces.data());
		fractal(conf, points, faces);
		return platonic(conf, points, faces);
	}

}
}
