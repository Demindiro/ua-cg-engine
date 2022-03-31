#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "math/point3d.h"

namespace shapes {
	constexpr void icosahedron(Point3D points[12]) {
		auto p = sqrtl(5) / 2;
		auto inv_p = 2 / sqrtl(5);
		points[0] = Point3D(0, 0,  p);
		points[1] = Point3D(0, 0, -p);
		// Beware, i has to be signed so i - 2 works properly
		for (int i = 0; i < 5; i++) {
			auto a = (i - 2) * 2 * M_PI / 5;
			auto b = a - M_PI / 5;
			points[2 + i] = Point3D(cos(a), sin(a),  0.5);
			points[7 + i] = Point3D(cos(b), sin(b), -0.5);
		}
		for (unsigned int i = 0; i < 12; i++) {
			// Normalize
			points[i].x *= inv_p;
			points[i].y *= inv_p;
			points[i].z *= inv_p;
		}
	}

	constexpr void icosahedron(Edge edges[30]) {
		for (unsigned int i = 0; i < 5; i++) {
			// Top & bottom "hat"
			edges[0 + i] = { 0, 2 + i };
			edges[5 + i] = { 1, 7 + i };
			// Top & bottom ring
			edges[10 + i] = { 2 + i, 2 + (i + 1) % 5 };
			edges[15 + i] = { 7 + i, 7 + (i + 1) % 5 };
			// Middle ring
			edges[20 + i] = { 2 + i, 7 + (i + 0) % 5 };
			edges[25 + i] = { 2 + i, 7 + (i + 1) % 5 };
		}
	}

	constexpr void icosahedron(Face faces[20]) {
		for (unsigned int i = 0; i < 5; i++) {
			// TODO double check order of vertices
			unsigned int j = (i + 1) % 5;
			// Top & bottom "hat"
			faces[0 + i] = { 0, 2 + i, 2 + j };
			faces[5 + i] = { 1, 7 + j, 7 + i };
			// Ring
			faces[10 + i] = { 2 + i, 7 + i, 7 + j };
			faces[15 + i] = { 2 + j, 2 + i, 7 + j };
		}
	}

	void icosahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure icosahedron(const FigureConfiguration &conf);

	void fractal_icosahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure fractal_icosahedron(const FigureConfiguration &conf);
}
