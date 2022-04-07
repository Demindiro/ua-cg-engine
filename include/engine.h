#pragma once

#include <array>
#include <cmath>
#include <vector>
#include "easy_image.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/matrix4d.h"

#ifdef __GNUC__
# define ALWAYS_INLINE inline __attribute__((always_inline))
#else
# define ALWAYS_INLINE inline
#endif
#define UNREACHABLE assert(!"unreachable")

namespace engine {

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

template<size_t n>
static inline Point2D tup_to_point2d(const std::array<double, n> &v) {
	static_assert(n >= 2);
	return { v[0], v[1] };
}

static inline Point2D tup_to_point2d(std::vector<double> v) {
	auto y = v.at(1), x = v[0];
	return { x, y };
}

static inline Point3D tup_to_point3d(std::vector<double> v) {
	// Call v.at(2) first since it allows us to elide checks for the
	// other positions.
	auto z = v.at(2), y = v[1], x = v[0];
	return Point3D(x, y, z);
}

template<size_t n>
static inline Point3D tup_to_point3d(const std::array<double, n> &v) {
	static_assert(n >= 3);
	return { v[0], v[1], v[2] };
}

static inline Vector3D tup_to_vector3d(std::vector<double> v) {
	// Ditto
	auto z = v.at(2), y = v[1], x = v[0];
	return Vector3D(x, y, z);
}

template<size_t n>
static inline Vector3D tup_to_vector3d(const std::array<double, n> &v) {
	static_assert(n >= 3);
	return { v[0], v[1], v[2] };
}

static constexpr inline double deg2rad(double a) {
	return a / 180 * M_PI;
}

static constexpr inline double pow_uint(double x, unsigned int y) {
	double v = 1;
	while (y > 0) {
		if ((y & 1) > 0) {
			v *= x;
		}
		x *= x;
		y >>= 1;
	}
	return v;
}

struct Rotation {
	double u, v;

	constexpr Rotation() : u(1), v(0) {}
	constexpr Rotation(double a) : u(std::cos(a)), v(std::sin(a)) {}
	constexpr Rotation(double u, double v) : u(u), v(v) {}
	constexpr Rotation(Vector2D a, Vector2D b) : u(NAN), v(NAN) {
		if (a == Vector2D() || b == Vector2D()) {
			u = 1;
			v = 0;
		} else {
			a = a.normalize();
			b = b.normalize();
			u = a.dot(b);
			v = std::sqrt(1 - u * u);
			v = a.cross(b) < 0 ? -v : v;
		}
	}

	constexpr Rotation inv() const {
		return { u, -v };
	}

	constexpr Rotation operator *(Rotation rhs) const {
		// (a + bi) * (e + fi) = (a*e - b*f) + (b*e + a*f)i
		return { u * rhs.u - v * rhs.v, v * rhs.u + u * rhs.v };
	}

	constexpr Rotation operator *=(Rotation rhs) {
		return *this = *this * rhs;
	}

	constexpr Matrix4D x() const {
		Matrix4D m;
		m(2, 2) = m(3, 3) = u;
		m(2, 3) = v;
		m(3, 2) = -m(2, 3);
		return m;
	}

	constexpr Matrix4D y() const {
		Matrix4D m;
		m(1, 1) = m(3, 3) = u;
		m(1, 3) = -v;
		m(3, 1) = -m(1, 3);
		return m;
	}

	constexpr Matrix4D z() const {
		Matrix4D m;
		m(1, 1) = m(2, 2) = u;
		m(1, 2) = v;
		m(2, 1) = -m(1, 2);
		return m;
	}
};

}
