#include "shapes/fractal.h"
#include <vector>
#include "ini_configuration.h"
#include "shapes.h"
#include "math/point3d.h"
#include "math/vector3d.h"

using namespace std;

namespace shapes {
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

	void fractal(vector<Point3D> &points, vector<Edge> &edges, double scale, unsigned int iterations) {
		auto old_points_count = points.size();
		fractal(points, 1 / scale, iterations);
		auto old_edges_size = edges.size();
		edges.reserve(edges.size() * points.size() / old_points_count);
		for (unsigned int i = old_points_count; i < points.size(); i += old_points_count) {
			for (size_t k = 0; k < old_edges_size; k++) {
				auto e = edges[k];
				edges.push_back({e.a + i, e.b + i});
			}
		}
	}

	void fractal(vector<Point3D> &points, vector<Face> &faces, double scale, unsigned int iterations) {
		auto old_points_count = points.size();
		fractal(points, 1 / scale, iterations);
		auto old_faces_size = faces.size();
		faces.reserve(faces.size() * points.size() / old_points_count);
		for (unsigned int i = old_points_count; i < points.size(); i += old_points_count) {
			for (size_t k = 0; k < old_faces_size; k++) {
				auto f = faces[k];
				faces.push_back({f.a + i, f.b + i, f.c + i});
			}
		}
	}

	void fractal(const FigureConfiguration &conf, vector<Point3D> &points, vector<Edge> &edges) {
		auto scale = conf.section["fractalScale"].as_double_or_die();
		auto iterations = (unsigned int)conf.section["nrIterations"].as_int_or_die();
		fractal(points, edges, scale, iterations);
	}

	void fractal(const FigureConfiguration &conf, vector<Point3D> &points, vector<Face> &faces) {
		auto scale = conf.section["fractalScale"].as_double_or_die();
		auto iterations = (unsigned int)conf.section["nrIterations"].as_int_or_die();
		fractal(points, faces, scale, iterations);
	}
}
