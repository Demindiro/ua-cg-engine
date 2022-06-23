#include "shapes/fractal.h"
#include <vector>
#include "ini_configuration.h"
#include "shapes.h"
#include "math/point3d.h"
#include "math/vector3d.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

/**
 * \brief Generate points to create a fractal.
 */
static void fractal(vector<Point3D> &points, vector<Vector3D> &normals, double inv_scale, unsigned int iterations) {
	// Operations:
	// - scale original points & determine shift
	// - place copy of original points at current points
	// - repeat
	
	// Shifts for 0th iteration
	// Shift is relative to current point
	vector<Vector3D> shift;
	shift.reserve(points.size());
	for (auto p : points) {
		shift.push_back(p.to_vector() * inv_scale);
	}

	// Iteration state
	vector<Point3D> cur_points(points);

	while (iterations --> 0) {
		// Scale & shift
		for (auto &p : points) {
			p = p.to_vector() * inv_scale;
		}
		// Place copies
		vector<Point3D> new_points;
		new_points.reserve(cur_points.size() * points.size());
		for (size_t k = 0; k < cur_points.size(); k++) {
			auto c = cur_points[k].to_vector();
			auto s = shift[k % shift.size()];
			for (size_t i = 0; i < points.size(); i++) {
				new_points.push_back(points[i] + c - s);
			}
		}
		// Repeat
		cur_points = new_points;
		for (auto &s : shift) {
			s *= inv_scale;
		}
	}

	// Copy normals
	size_t old_ns = normals.size();
	size_t ns = old_ns * cur_points.size() / points.size();
	normals.reserve(ns);
	while (normals.size() < ns) {
		normals.push_back(normals[normals.size() - old_ns]);
	}

	points = cur_points;
}

void fractal(double scale, unsigned int iterations, EdgeShape &f) {
	auto old_points_count = f.points.size();
	vector<Vector3D> normals;
	fractal(f.points, normals, 1 / scale, iterations);
	auto old_edges_size = f.edges.size();
	f.edges.reserve(f.edges.size() * f.points.size() / old_points_count);
	for (unsigned int i = old_points_count; i < f.points.size(); i += old_points_count) {
		for (size_t k = 0; k < old_edges_size; k++) {
			auto e = f.edges[k];
			f.edges.push_back({e.a + i, e.b + i});
		}
	}
}

void fractal(double scale, unsigned int iterations, FaceShape &f) {
	auto old_points_count = f.points.size();
	fractal(f.points, f.normals, 1 / scale, iterations);
	auto old_faces_size = f.faces.size();
	f.faces.reserve(f.faces.size() * f.points.size() / old_points_count);
	for (unsigned int i = old_points_count; i < f.points.size(); i += old_points_count) {
		for (size_t k = 0; k < old_faces_size; k++) {
			auto e = f.faces[k];
			f.faces.push_back({e.a + i, e.b + i, e.c + i});
		}
	}
}

void fractal(const Configuration &conf, const ShapeTemplateAny &shape, EdgeShape &f) {
	auto scale = conf.section["fractalScale"].as_double_or_die();
	auto iterations = (unsigned int)conf.section["nrIterations"].as_int_or_die();
	f.points = { shape.points, shape.points + shape.points_size };
	f.edges = { shape.edges, shape.edges + shape.edges_size };
	fractal(scale, iterations, f);
}

void fractal(const Configuration &conf, const ShapeTemplateAny &shape, FaceShape &f) {
	auto scale = conf.section["fractalScale"].as_double_or_die();
	auto iterations = (unsigned int)conf.section["nrIterations"].as_int_or_die();
	f.points = { shape.points, shape.points + shape.points_size };
	f.faces = { shape.faces, shape.faces + shape.faces_size };
	f.normals = conf.point_normals
		? vector<Vector3D>(shape.point_normals, shape.point_normals + shape.point_normals_size)
		: vector<Vector3D>(shape.face_normals, shape.face_normals + shape.face_normals_size);
	fractal(scale, iterations, f);
}

}
}
