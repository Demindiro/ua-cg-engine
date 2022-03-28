#include "shapes/cube.h"
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/fractal.h"
#include "vector3d.h"

using namespace std;

namespace shapes {

	static void cube(Point3D points[8]) {
		points[0] = {  1,  1,  1 };
		points[1] = {  1,  1, -1 };
		points[2] = {  1, -1,  1 };
		points[3] = {  1, -1, -1 };
		points[4] = { -1,  1,  1 };
		points[5] = { -1,  1, -1 };
		points[6] = { -1, -1,  1 };
		points[7] = { -1, -1, -1 };
	}

	static void cube(Edge edges[12]) {
		// X
		edges[0] = { 0, 4 };
		edges[1] = { 1, 5 };
		edges[2] = { 2, 6 };
		edges[3] = { 3, 7 };
		// Y
		edges[4] = { 0, 2 };
		edges[5] = { 1, 3 };
		edges[6] = { 4, 6 };
		edges[7] = { 5, 7 };
		// Z
		edges[ 8] = { 0, 1 };
		edges[ 9] = { 2, 3 };
		edges[10] = { 4, 5 };
		edges[11] = { 6, 7 };
	}

	static void cube(Face faces[12]) {
		// TODO fix order of vertices so culling works properly
		// X
		faces[0] = { 0, 1, 2 };
		faces[1] = { 3, 1, 2 };
		faces[2] = { 4, 5, 6 };
		faces[3] = { 7, 5, 6 };
		// Y
		faces[4] = { 0, 1, 4 };
		faces[5] = { 5, 1, 4 };
		faces[6] = { 2, 3, 6 };
		faces[7] = { 7, 3, 6 };
		// Z
		faces[ 8] = { 0, 2, 4 };
		faces[ 9] = { 2, 6, 4 };
		faces[10] = { 1, 3, 5 };
		faces[11] = { 3, 7, 5 };
	}

	void cube(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Point3D points[8];
		Edge edges[12];
		cube(points);
		cube(edges);
		platonic(conf, mat_project, lines, points, 8, edges, 12);
	}

	TriangleFigure cube(ini::Section &conf, Matrix &mat_project) {
		vector<Point3D> points(8);
		vector<Face> faces(12);
		cube(points.data());
		cube(faces.data());
		return platonic(conf, mat_project, points, faces);
	}

	void fractal_cube(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		vector<Point3D> points(8);
		vector<Edge> edges(12);
		cube(points.data());
		cube(edges.data());
		fractal(conf, points, edges);
		platonic(conf, mat_project, lines, points, edges);
	}

	TriangleFigure fractal_cube(ini::Section &conf, Matrix &mat_project) {
		vector<Point3D> points(8);
		vector<Face> faces(12);
		cube(points.data());
		cube(faces.data());
		fractal(conf, points, faces);
		return platonic(conf, mat_project, points, faces);
	}
}
