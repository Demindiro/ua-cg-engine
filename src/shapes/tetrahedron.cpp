#include "shapes/tetrahedron.h"
#include "shapes.h"
#include "shapes/fractal.h"

using namespace std;

namespace shapes {
	static void tetrahedron(Vector3D points[4]) {
		points[0] = Vector3D::point( 1, -1, -1);
		points[1] = Vector3D::point(-1,  1, -1);
		points[2] = Vector3D::point( 1,  1,  1);
		points[3] = Vector3D::point(-1, -1,  1);
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
		faces[3] = { 1, 2, 3 };
	}

	void tetrahedron(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[4];
		Edge edges[6];
		tetrahedron(points);
		tetrahedron(edges);
		platonic(conf, mat_project, lines, points, 4, edges, 6);
	}

	void tetrahedron(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		Vector3D points[4];
		Face faces[4];
		tetrahedron(points);
		tetrahedron(faces);
		platonic(conf, mat_project, triangles, points, 4, faces, 4);
	}

	void fractal_tetrahedron(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		vector<Vector3D> points(4);
		vector<Edge> edges(6);
		tetrahedron(points.data());
		tetrahedron(edges.data());
		fractal(conf, points, edges);
		platonic(conf, mat_project, lines, points, edges);
	}

	void fractal_tetrahedron(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		vector<Vector3D> points(4);
		vector<Face> faces(4);
		tetrahedron(points.data());
		tetrahedron(faces.data());
		fractal(conf, points, faces);
		platonic(conf, mat_project, triangles, points, faces);
	}
}
