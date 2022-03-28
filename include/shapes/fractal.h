#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void fractal(std::vector<Point3D> &points, std::vector<Edge> &edges, double scale, unsigned int iterations);

	void fractal(std::vector<Point3D> &points, std::vector<Face> &faces, double scale, unsigned int iterations);

	void fractal(const FigureConfiguration &, std::vector<Point3D> &points, std::vector<Edge> &edges);

	void fractal(const FigureConfiguration &, std::vector<Point3D> &points, std::vector<Face> &faces);
}
