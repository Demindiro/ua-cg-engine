#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "math/point3d.h"

namespace shapes {
	void icosahedron(Point3D points[12]);

	void icosahedron(Edge edges[30]);

	void icosahedron(Face faces[20]);

	void icosahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure icosahedron(const FigureConfiguration &conf);

	void fractal_icosahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure fractal_icosahedron(const FigureConfiguration &conf);
}
