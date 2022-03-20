#pragma once

#include <cmath>
#include <vector>
#include "easy_image.h"
#include "vector3d.h"

class TypeException : public std::exception {
	std::string type;
	
public:
	TypeException(const std::string type) throw();

	virtual const char *what() const throw();
};

// Returns -1 if lower than 0, otherwise 1.
static inline int signum_or_one(int x) {
	return x < 0 ? -1 : 1;
}

// Round up to +inf
//
// This is faster than std::round, which accounts for sign and
// sets errno (without -ffast-math et al.).
static inline double round_up(double x) {
	return (long long)(x + 0.5);
}

static inline img::Color tup_to_color(std::vector<double> v) {
	return img::Color(round_up(v.at(0) * 255), round_up(v.at(1) * 255), round_up(v.at(2) * 255));
}

static inline Vector3D tup_to_point3d(std::vector<double> v) {
	// Call v.at(2) first since it allows us to elide checks for the
	// other positions.
	auto z = v.at(2), y = v[1], x = v[0];
	return Vector3D::point(x, y, z);
}

struct Rotation {
	double u, v;

	constexpr Rotation() : u(1), v(0) {}
	constexpr Rotation(double a) : u(std::cos(a)), v(std::sin(a)) {}
	constexpr Rotation(double u, double v) : u(u), v(v) {}

	constexpr Rotation inv() const {
		return { u, -v };
	}

	// TODO make matrix constructor & accessors constexpr
	Matrix x() const {
		Matrix m;
		m(2, 2) = m(3, 3) = u;
		m(2, 3) = v;
		m(3, 2) = -m(2, 3);
		return m;
	}

	Matrix y() const {
		Matrix m;
		m(1, 1) = m(3, 3) = u;
		m(1, 3) = -v;
		m(3, 1) = -m(1, 3);
		return m;
	}

	Matrix z() const {
		Matrix m;
		m(1, 1) = m(2, 2) = u;
		m(1, 2) = v;
		m(2, 1) = -m(1, 2);
		return m;
	}
};
