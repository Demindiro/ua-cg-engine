#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace shapes {
	struct Edge {
		unsigned int a, b;

		bool operator<(const Edge &rhs) const {
			auto ll = std::min(a, b), lh = std::max(a, b);
			auto rl = std::min(rhs.a, rhs.b), rh = std::max(rhs.a, rhs.b);
			return ll == rl ? lh < rh : ll < rl;
		}
	};

	struct Face {
		unsigned int a, b, c;
	};

	struct TriangleFigure {
		std::vector<Point3D> points;
		std::vector<Face> faces;
		Color ambient;
		Color diffuse;
		Color specular;
		double reflection;
	};

	Matrix transform_from_conf(ini::Section &conf, Matrix &projection);

	img::Color color_from_conf(ini::Section &conf);

	void platonic(ini::Section &conf, Matrix &project, std::vector<Line3D> &lines, Point3D *points, unsigned int points_len, Edge *edges, unsigned int edges_len);

	inline void platonic(ini::Section &conf, Matrix &project, std::vector<Line3D> &lines, std::vector<Point3D> points, std::vector<Edge> edges) {
		platonic(conf, project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	TriangleFigure platonic(ini::Section &conf, Matrix &project, std::vector<Point3D> points, std::vector<Face> faces);

	img::EasyImage wireframe(const ini::Configuration &, bool with_z);

	img::EasyImage triangles(const ini::Configuration &);

	img::EasyImage draw(const std::vector<TriangleFigure> &figures, unsigned int size, Color background);
}
