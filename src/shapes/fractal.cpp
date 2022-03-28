#include "shapes/fractal.h"
#include <vector>
#include "ini_configuration.h"
#include "shapes.h"

using namespace std;

namespace shapes {
	/**
	 * \brief Generate points to create a fractal.
	 */
	static void fractal(vector<Vector3D> &points, double inv_scale, unsigned int iterations) {
		// Operations:
		// - scale original points & determine shift
		// - place copy of original points at current points
		// - repeat
		
		// Shifts for 0th iteration
		// Shift is relative to current point
		vector<Vector3D> shift;
		shift.reserve(points.size());
		for (auto p : points) {
			shift.push_back(p * inv_scale);
		}

		// Iteration state
		vector<Vector3D> cur_points(points);

		while (iterations --> 0) {
			// Scale & shift
			for (auto &p : points) {
				p *= inv_scale;
			}
			// Place copies
			vector<Vector3D> new_points;
			new_points.reserve(cur_points.size() * points.size());
			for (size_t k = 0; k < cur_points.size(); k++) {
				auto c = cur_points[k];
				auto s = shift[k % shift.size()];
				for (size_t i = 0; i < points.size(); i++) {
					new_points.push_back(c - s + points[i]);
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

	void fractal(vector<Vector3D> &points, vector<Edge> &edges, double scale, unsigned int iterations) {
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

	void fractal(vector<Vector3D> &points, vector<Face> &faces, double scale, unsigned int iterations) {
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

	void fractal(ini::Section &conf, vector<Vector3D> &points, vector<Edge> &edges) {
		auto scale = conf["fractalScale"].as_double_or_die();
		auto iterations = (unsigned int)conf["nrIterations"].as_int_or_die();
		fractal(points, edges, scale, iterations);
	}

	void fractal(ini::Section &conf, vector<Vector3D> &points, vector<Face> &faces) {
		auto scale = conf["fractalScale"].as_double_or_die();
		auto iterations = (unsigned int)conf["nrIterations"].as_int_or_die();
		fractal(points, faces, scale, iterations);
	}
}
