#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void fractal(std::vector<Vector3D> &points, std::vector<Edge> &edges, double scale, unsigned int iterations);

	void fractal(std::vector<Vector3D> &points, std::vector<Face> &faces, double scale, unsigned int iterations);

	void fractal(ini::Section &, std::vector<Vector3D> &points, std::vector<Edge> &edges);

	void fractal(ini::Section &, std::vector<Vector3D> &points, std::vector<Face> &faces);
}
