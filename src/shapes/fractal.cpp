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
static void fractal(vector<Point3D> &points, double inv_scale, unsigned int iterations) {
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
	points = cur_points;
}

/**
 * \brief Generate points to create a fractal.
 */
static void fractal(const ShapeTemplateAny &shape, double inv_scale, unsigned int iterations, vector<FaceShape> &fv) {
	// Operations:
	// - determine points iteratively
	// - place scaled copies at points

	inv_scale = 1 / inv_scale;

	// Shifts for 0th iteration
	// Shift is relative to current point
	vector<Vector3D> shift;
	shift.reserve(shape.points_size);
	for (size_t i = 0; i < shape.points_size; i++) {
		shift.push_back(shape.points[i].to_vector() * (1 - inv_scale));
	}

	double total_scale = 1.0;

	// Iteration state
	vector<Point3D> points({ Point3D() });

	while (iterations --> 0) {
		// Place points
		vector<Point3D> new_points;
		new_points.reserve(points.size() * points.size());
		for (size_t k = 0; k < points.size(); k++) {
			for (auto &s : shift) {
				new_points.push_back(points[k] + s);
			}
		}
		for (auto &s : shift) {
			s *= inv_scale;
		}
		// Repeat
		points = new_points;
		// Scale & shift
		total_scale *= inv_scale;
	}

	// Place copies
	fv.reserve(points.size());
	for (auto p : points) {
		fv.push_back(FaceShape(shape));
		for (auto &q : fv.back().points) {
			q = p + q.to_vector() * total_scale;
		}
		fv.back().center = p;
	}
}

void fractal(double scale, unsigned int iterations, EdgeShape &f) {
	auto old_points_count = f.points.size();
	fractal(f.points, 1 / scale, iterations);
	auto old_edges_size = f.edges.size();
	f.edges.reserve(f.edges.size() * f.points.size() / old_points_count);
	for (unsigned int i = old_points_count; i < f.points.size(); i += old_points_count) {
		for (size_t k = 0; k < old_edges_size; k++) {
			auto e = f.edges[k];
			f.edges.push_back({e.a + i, e.b + i});
		}
	}
}

/*
void fractal(const ShapeTemplateAny &shape, double scale, unsigned int iterations, vector<FaceShape> &fv) {
	auto old_points_count = f.points.size();
	fractal(f.points, 1 / scale, iterations);
	auto old_faces_size = f.faces.size();
	f.faces.reserve(f.faces.size() * f.points.size() / old_points_count);
	for (unsigned int i = old_points_count; i < f.points.size(); i += old_points_count) {
		for (size_t k = 0; k < old_faces_size; k++) {
			auto e = f.faces[k];
			f.faces.push_back({e.a + i, e.b + i, e.c + i});
		}
	}
	fractal(f, 1 / scale, iterations);
}
*/

void fractal(const ini::Section &conf, const ShapeTemplateAny &shape, EdgeShape &f) {
	auto scale = conf["fractalScale"].as_double_or_die();
	auto iterations = (unsigned int)conf["nrIterations"].as_int_or_die();
	f.points = { shape.points, shape.points + shape.points_size };
	f.edges = { shape.edges, shape.edges + shape.edges_size };
	fractal(scale, iterations, f);
}

void fractal(const ini::Section &conf, const ShapeTemplateAny &shape, vector<FaceShape> &fv) {
	auto scale = conf["fractalScale"].as_double_or_die();
	auto iterations = (unsigned int)conf["nrIterations"].as_int_or_die();
	//f.points = { shape.points, shape.points + shape.points_size };
	//f.faces = { shape.faces, shape.faces + shape.faces_size };
	fractal(shape, scale, iterations, fv);
}

}
}
