#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void icosahedron(Point3D points[12]);

	void icosahedron(Edge edges[30]);

	void icosahedron(Face faces[20]);

	void icosahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure icosahedron(ini::Section &conf, Matrix &mat_project);

	void fractal_icosahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure fractal_icosahedron(ini::Section &conf, Matrix &mat_project);
}
