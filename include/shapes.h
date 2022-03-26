#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace shapes {
	struct Edge {
		int a, b;

		bool operator<(const Edge &rhs) const {
			auto ll = std::min(a, b), lh = std::max(a, b);
			auto rl = std::min(rhs.a, rhs.b), rh = std::max(rhs.a, rhs.b);
			return ll == rl ? lh < rh : ll < rl;
		}
	};

	struct Face {
		int a, b, c;
	};

	Matrix transform_from_conf(ini::Section &conf, Matrix &projection);

	img::Color color_from_conf(ini::Section &conf);

	void platonic(ini::Section &conf, Matrix &project, std::vector<Line3D> &lines, Vector3D *points, int points_len, Edge *edges, int edges_len);

	void platonic(ini::Section &conf, Matrix &project, std::vector<Triangle3D> &triangles, Vector3D *points, int points_len, Face *faces, int faces_len);

	img::EasyImage wireframe(const ini::Configuration &, bool with_z);

	img::EasyImage triangles(const ini::Configuration &);
}
