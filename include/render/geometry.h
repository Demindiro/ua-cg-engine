#pragma once

#include <cmath>
#include "math/matrix2d.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "render/triangle.h"
#include "util.h"

namespace engine {
namespace render {

struct Frustum {
	double near, far;
	double fov, aspect;

	void clip(TriangleFigure &f) const;

	/**
	 * \brief Determine the perspective scaling factor d for this frustum for a given width.
	 */
	double perspective_factor(double width) const {
		return 2 * std::atan(fov / 2) / width;
	}
};

constexpr Point2D project(Point3D p) {
	assert(p.z != 0 && "division by 0");
	return { p.x / -p.z, p.y / -p.z };
}

constexpr Point2D project(Point3D p, double d, Vector2D offset) {
	return Point2D(project(p).to_vector() * d + offset);
}

/**
 * \brief Determine P and Q interpolation factors for BA and CA respectively for
 * a triangle ABC.
 */
ALWAYS_INLINE Vector2D calc_pq(Point3D a, Point3D b, Point3D c, Point3D point) {
	auto ba = b - a;
	auto ca = c - a;
	auto pa = point - a;

	Matrix2D m_xy {{ ba.x, ca.x }, { ba.y, ca.y }};
	Matrix2D m_yz {{ ba.y, ca.y }, { ba.z, ca.z }};
	Matrix2D m_xz {{ ba.x, ca.x }, { ba.z, ca.z }};

	Vector2D p_xy { pa.x, pa.y };
	Vector2D p_yz { pa.y, pa.z };
	Vector2D p_xz { pa.x, pa.z };

	// Find out which matrix will result in the least precision loss
	// i.e. find the matrix with the highest determinant.
	auto d_xy = std::abs(m_xy.determinant());
	auto d_yz = std::abs(m_yz.determinant());
	auto d_xz = std::abs(m_xz.determinant());

	Matrix2D m;
	Vector2D p;

	if (d_xy > d_yz) {
		if (d_xy > d_xz) {
			m = m_xy;
			p = p_xy;
		} else {
			m = m_xz;
			p = p_xz;
		}
	} else {
		if (d_yz > d_xz) {
			m = m_yz;
			p = p_yz;
		} else {
			m = m_xz;
			p = p_xz;
		}
	}

	return p * m.inv();
}

/**
 * \brief Interpolate between 3 points
 */
template<typename T>
static ALWAYS_INLINE T interpolate(T a, T b, T c, Vector2D pq) {
	return (1 - pq.x - pq.y) * a + pq.x * b + pq.y * c;
}

}
}
