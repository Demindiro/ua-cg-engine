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
