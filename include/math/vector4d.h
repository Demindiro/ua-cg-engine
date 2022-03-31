#pragma once

#include <cassert>
#include <ostream>

struct Vector4D {
	double x, y, z, w;

	constexpr Vector4D() : x(0), y(0), z(0), w(0) {}

	constexpr double operator [](unsigned int row) const {
		assert(row < 4);
		double e[4] = { x, y, z, w };
		return e[row];
	}

	constexpr double &operator [](unsigned int row) {
		assert(row < 4);
		double *e[4] = { &x, &y, &z, &w };
		return *e[row];
	}

	constexpr Vector4D(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

	constexpr double dot(const Vector4D &v) const {
		return (x * v.x + y * v.y) + (z * v.z + w * v.w);
	}
};

std::ostream &operator <<(std::ostream &, const Vector4D &);
