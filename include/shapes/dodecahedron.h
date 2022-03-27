#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace shapes {
	void dodecahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	void dodecahedron(ini::Section &conf, Matrix &mat_project, std::vector<Triangle3D> &triangles);

	void fractal_dodecahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	void fractal_dodecahedron(ini::Section &conf, Matrix &mat_project, std::vector<Triangle3D> &triangles);
}
