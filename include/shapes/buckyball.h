#pragma once

#include <array>
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/icosahedron.h"

namespace engine {
namespace shapes {

template<size_t n>
constexpr void _buckyball_gen_faces(
	const std::array<Point3D, 60> &points,
	Point3D center,
	std::array<render::Face, 116> &faces,
	size_t &face_i
) {
	const double limit = 0.3505; // "exact" length is 0.350487
	// Gather points
	std::array<unsigned int, n> qa = {};
	size_t qi = 0;
	for (unsigned int i = 0; i < points.size(); i++) {
		if (center.distance_to(points[i]) < limit) {
			qa[qi++] = i;
		}
	}
	// Ensure points are grouped such that any two neighbours
	// are within the limit. Start with the first and rotate
	// until it fits. Repeat for next to end.
	for (size_t i = 0; i < n - 1; i++) {
		while (points[qa[i]].distance_to(points[qa[i + 1]]) > limit) {
			// std::rotate is only constexpr in C++20 :(
			auto temp = qa[i + 1];
			for (size_t k = i + 1; k < n - 1; k++) {
				qa[k] = qa[k + 1];
			}
			qa[n - 1] = temp;
		}
	}
	// Ensure that the face points outwards. If not, just reverse
	// the points.
	auto a = points[qa[0]];
	auto b = points[qa[1]];
	auto c = points[qa[2]];
	if ((b - a).cross(c - a).dot(a - Point3D()) < 0) {
		unsigned int t = -1;
		t = qa[0], qa[0] = qa[4], qa[4] = t;
		t = qa[1], qa[1] = qa[3], qa[3] = t;
	}
	for (size_t i = 1; i < n - 1; i++) {
		faces[face_i++] = { qa[0], qa[i], qa[i + 1] };
	}
}

constexpr ShapeTemplate<60, 90, 116> buckyball(
	[]() constexpr {
		// Trisect the edges of an icosahedron
		// The two middle points are the points of the buckyball
		std::array<Point3D, 60> points = {};
		for (int i = 0; i < 30; i++) {
			auto e = icosahedron.edges[i];
			auto a = icosahedron.points[e.a];
			auto b = icosahedron.points[e.b];
			points[i * 2 + 0] = a.interpolate(b, 2.0 / 3);
			points[i * 2 + 1] = a.interpolate(b, 1.0 / 3);
		}
		return points;
	},
	[](auto &points) constexpr {
		std::array<render::Edge, 90> edges = {};
		const double limit = 0.3505; // "exact" length is 0.350487
		unsigned int i = 0;
		for (unsigned int u = 0; u < 60; u++) {
			auto p = points[u];
			for (unsigned int v = 0; v < u; v++) {
				auto q = points[v];
				if (p.distance_to(q) < limit) {
					assert(i < 90);
					edges[i++] = { u, v };
				}
			}
		}
		return edges;
	},
	[](auto &points, auto) constexpr {
		std::array<render::Face, 116> faces = {};
		size_t face_i = 0;
		// Generate 5-gons
		for (auto p : icosahedron.points) {
			_buckyball_gen_faces<5>(points, p, faces, face_i);
		}
		// Generate 6-gons
		for (auto &f : icosahedron.faces) {
			auto &ico_p = icosahedron.points;
			auto mid = Point3D::center({ ico_p[f.a], ico_p[f.b], ico_p[f.c] });
			_buckyball_gen_faces<6>(points, mid, faces, face_i);
		}
		return faces;
	}
);

}
}
