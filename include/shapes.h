#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <ostream>
#include <vector>
#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"
#include "lines.h"
#include "math/matrix4d.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/vector3d.h"
#include "render/color.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

	struct Edge {
		unsigned int a, b;

		bool operator<(const Edge &rhs) const {
			auto ll = std::min(a, b), lh = std::max(a, b);
			auto rl = std::min(rhs.a, rhs.b), rh = std::max(rhs.a, rhs.b);
			return ll == rl ? lh < rh : ll < rl;
		}
	};

	struct FigureConfiguration {
		ini::Section &section;
		Matrix4D eye;
		bool with_lighting;
		bool face_normals;
	};

	Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection);

	render::Color color_from_conf(const ini::Section &conf);

	/**
	 * \brief Generic face normal calculator.
	 */
	std::vector<Vector3D> calculate_face_normals(const std::vector<Point3D> &points, const std::vector<render::Face> &faces);

	void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, const Point3D *points, unsigned int points_len, const Edge *edges, unsigned int edges_len);
	
	template<size_t points_size, size_t edges_size>
	inline void platonic(
		const FigureConfiguration &conf,
		std::vector<Line3D> &lines,
		const std::array<Point3D, points_size> &points,
		const std::array<Edge, edges_size> &edges
	) {
		platonic(conf, lines, points.data(), points_size, edges.data(), edges_size);
	}

	inline void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, std::vector<Point3D> points, std::vector<Edge> edges) {
		platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
	}
	
	template<size_t points_size, size_t faces_size>
	inline render::TriangleFigure platonic(
		const FigureConfiguration &conf,
		const std::array<Point3D, points_size> &points,
		const std::array<render::Face, faces_size> &faces
	) {
		return platonic(conf, { points.begin(), points.end() }, { faces.begin(), faces.end() });
	}

	render::TriangleFigure platonic(const FigureConfiguration &conf, std::vector<Point3D> points, std::vector<render::Face> faces);

	img::EasyImage wireframe(const ini::Configuration &, bool with_z);

	img::EasyImage triangles(const ini::Configuration &, bool with_lighting);

}
}
