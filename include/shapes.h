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

struct Configuration {
	const ini::Section &section;
	bool point_normals;
};

constexpr void calculate_face_normals(
	const Point3D *points,
	const render::Face *faces,
	unsigned int faces_count,
	Vector3D *normals
) {
	for (unsigned int i = 0; i < faces_count; i++) {
		auto a = points[faces[i].a];
		auto b = points[faces[i].b];
		auto c = points[faces[i].c];
		normals[i] = (b - a).cross(c - a).normalize();
	}
}

constexpr void calculate_platonic_point_normals(
	const Point3D *points,
	unsigned int points_count,
	Vector3D *normals
) {
	for (unsigned int i = 0; i < points_count; i++) {
		normals[i] = (points[i] - Point3D()).normalize();
	}
}

template<unsigned int points_count, unsigned int edges_count, unsigned int faces_count>
struct ShapeTemplate {
	std::array<Point3D, points_count> points = {};
	std::array<render::Edge, edges_count> edges = {};
	std::array<render::Face, faces_count> faces = {};
	std::array<Vector3D, faces_count> face_normals = {};
	std::array<Vector3D, points_count> point_normals = {};

	constexpr ShapeTemplate<points_count, edges_count, faces_count>(
		std::array<Point3D, points_count> points,
		std::array<render::Edge, edges_count> edges,
		std::array<render::Face, faces_count> faces,
		std::array<Vector3D, faces_count> face_normals,
		std::array<Vector3D, points_count> point_normals
	) : points(points) , edges(edges), faces(faces),
	face_normals(face_normals), point_normals(point_normals) {}

	constexpr ShapeTemplate<points_count, edges_count, faces_count>(
		std::array<Point3D, points_count> points,
		std::array<render::Edge, edges_count> edges,
		std::array<render::Face, faces_count> faces
	) : points(points) , edges(edges), faces(faces) {
		calculate_face_normals(points.data(), faces.data(), faces.size(), face_normals.data());
		calculate_platonic_point_normals(points.data(), points.size(), point_normals.data());
	}

	template<typename F, typename G, typename H>
	constexpr ShapeTemplate<points_count, edges_count, faces_count>(F f, G g, H h) {
		points = f();
		edges = g(points);
		faces = h(points, edges);
		calculate_face_normals(points.data(), faces.data(), faces.size(), face_normals.data());
		calculate_platonic_point_normals(points.data(), points.size(), point_normals.data());
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
};

struct FaceShape {
	std::vector<Point3D> points;
	std::vector<Vector3D> normals;
	std::vector<Point2D> uvs;
	std::vector<render::Face> faces;

	FaceShape() {}

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	FaceShape(const ShapeTemplate<points_c, edges_c, faces_c> &t, bool point_normals)
		: points({ t.points.begin(), t.points.end() })
		, faces({ t.faces.begin(), t.faces.end() })
	{
		if (point_normals) {
			normals = { t.point_normals.begin(), t.point_normals.end() };
		} else {
			normals = { t.face_normals.begin(), t.face_normals.end() };
		}
	}
};

struct ShapeTemplateAny {
	const Point3D *points;
	const render::Edge *edges;
	const render::Face *faces;
	const Vector3D *face_normals;
	const Vector3D *point_normals;
	unsigned int points_size, edges_size, faces_size, face_normals_size, point_normals_size;

	template<unsigned int points_c, unsigned int edges_c, unsigned int faces_c>
	ShapeTemplateAny(const ShapeTemplate<points_c, edges_c, faces_c> &shape) 
		: points(shape.points.data())
		, edges(shape.edges.data())
		, faces(shape.faces.data())
		, face_normals(shape.face_normals.data())
		, point_normals(shape.point_normals.data())
		, points_size(shape.points.size())
		, edges_size(shape.edges.size())
		, faces_size(shape.faces.size())
		, face_normals_size(shape.face_normals.size())
		, point_normals_size(shape.point_normals.size())
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

struct Material {
	std::optional<render::Texture> texture;
	render::Color ambient, diffuse, specular;
	double reflection;
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
