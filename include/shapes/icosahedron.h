#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void icosahedron(Vector3D points[12]);

	void icosahedron(Edge edges[30]);

	void icosahedron(Face faces[20]);

	void icosahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	void icosahedron(ini::Section &conf, Matrix &mat_project, std::vector<Triangle3D> &triangles);

	void fractal_icosahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	void fractal_icosahedron(ini::Section &conf, Matrix &mat_project, std::vector<Triangle3D> &triangles);
}
