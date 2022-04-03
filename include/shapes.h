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
#include "render/lines.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

template<unsigned int points_count, unsigned int edges_count, unsigned int faces_count>
struct ShapeTemplate {
	std::array<Point3D, points_count> points = {};
	std::array<render::Edge, edges_count> edges = {};
	std::array<render::Face, faces_count> faces = {};

	constexpr ShapeTemplate<points_count, edges_count, faces_count>(
		std::array<Point3D, points_count> points,
		std::array<render::Edge, edges_count> edges,
		std::array<render::Face, faces_count> faces
	) : points(points), edges(edges), faces(faces) {}

	template<typename F, typename G, typename H>
	constexpr ShapeTemplate<points_count, edges_count, faces_count>(F f, G g, H h) {
		points = f();
		edges = g(points);
		faces = h(points, edges);
	}
};

struct EdgeShape {
	std::vector<Point3D> points;
	std::vector<render::Edge> edges;

	EdgeShape() {}

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	EdgeShape(const ShapeTemplate<points_c, edges_c, faces_c> &t)
		: points({ t.points.begin(), t.points.end() })
		, edges({ t.edges.begin(), t.edges.end() })
	{}

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	EdgeShape &operator =(const ShapeTemplate<points_c, edges_c, faces_c> &t) {
		points = { t.points.begin(), t.points.end() };
		edges = { t.edges.begin(), t.edges.end() };
		return *this;
	}
};

struct FaceShape {
	std::vector<Point3D> points;
	std::vector<render::Face> faces;

	FaceShape() {}

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	FaceShape(const ShapeTemplate<points_c, edges_c, faces_c> &t)
		: points({ t.points.begin(), t.points.end() })
		, faces({ t.faces.begin(), t.faces.end() })
	{}

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	FaceShape &operator =(const ShapeTemplate<points_c, edges_c, faces_c> &t) {
		points = { t.points.begin(), t.points.end() };
		faces = { t.faces.begin(), t.faces.end() };
		return *this;
	}
};

struct ShapeTemplateAny {
	const Point3D *points;
	const render::Edge *edges;
	const render::Face *faces;
	unsigned int points_size, edges_size, faces_size;

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	ShapeTemplateAny(const ShapeTemplate<points_c, edges_c, faces_c> &shape) 
		: points(shape.points.data())
		, edges(shape.edges.data())
		, faces(shape.faces.data())
		, points_size(shape.points.size())
		, edges_size(shape.edges.size())
		, faces_size(shape.faces.size())
	{}

	ShapeTemplateAny(const EdgeShape &shape)
		: points(shape.points.data())
		, edges(shape.edges.data())
		, faces(nullptr)
		, points_size(shape.points.size())
		, edges_size(shape.edges.size())
		, faces_size(0)
	{}
};

Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection);

render::Color color_from_conf(const ini::Section &conf);

/**
 * \brief Generic face normal calculator.
 */
std::vector<Vector3D> calculate_face_normals(const std::vector<Point3D> &points, const std::vector<render::Face> &faces);

img::EasyImage wireframe(const ini::Configuration &, bool with_z);

img::EasyImage triangles(const ini::Configuration &, bool with_lighting);

}
}
