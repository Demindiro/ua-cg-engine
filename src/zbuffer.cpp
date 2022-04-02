#include "zbuffer.h"
#include <algorithm>
#include "engine.h"
#include "math/point3d.h"
#include "math/vector3d.h"

namespace engine {

using namespace std;

template<typename F>
void ZBuffer::triangle(
	Point3D a, Point3D b, Point3D c,
	double d, Vector2D offset,
	double bias,
	F callback
) {
	// Find repricoral Z-values first, which require unprojected points
	// Optimized version of 1 / (3 * a.z) + 1 / (3 * b.z) + 1 / (3 * c.z)
	// The former will emit 3 div instructions even with -Ofast
	double inv_g_z = (b.z * c.z + a.z * c.z + a.z * b.z) / (3 * a.z * b.z * c.z);
	double dzdx, dzdy;
	{
		auto w = (b - a).cross(c - a);
		auto dk = d * w.dot(a.to_vector());
		dzdx = -w.x / dk;
		dzdy = -w.y / dk;
	}
	inv_g_z *= bias;
	
	// Project
	a.x = a.x * (d / -a.z) + offset.x, a.y = a.y * (d / -a.z) + offset.y;
	b.x = b.x * (d / -b.z) + offset.x, b.y = b.y * (d / -b.z) + offset.y;
	c.x = c.x * (d / -c.z) + offset.x, c.y = c.y * (d / -c.z) + offset.y;

	// These center coordaintes must be projected.
	double g_x = (a.x + b.x + c.x) / 3;
	double g_y = (a.y + b.y + c.y) / 3;

	// Sort triangles based on Y (ay <= by <= cy)
	if (b.y < a.y) { swap(b, a); };
	if (c.y < a.y) { swap(c, a); };
	if (c.y < b.y) { swap(c, b); };

	// Determine if b is left or right to avoid min max inside loop
	double p = (b.y - a.y) / (c.y - a.y);
	assert(0 <= p);
	assert(p <= 1);
	bool b_left = b.x < a.x * (1 - p) + c.x * p;

	auto f = [](auto y, Point3D p, Point3D q) {
		// Find intersections
		return q.x + (p.x - q.x) * (y - q.y) / (p.y - q.y);
	};

	// Start from bottom to middle
	{
		// 1.0 --> round(1.5) --> 2.0
		// 1.9 --> floor(2.9) --> 2.0
		unsigned int from_y = (unsigned int)a.y + 1;
		// 1.0 --> round(0.5) --> 1.0
		// 1.0 --> floor(1.0) --> 1.0
		unsigned int to_y = b.y;
	
		for (unsigned int y = from_y; y <= to_y; y++) {
			// Find intersections
			double ab = f(y, a, b), ac = f(y, a, c);

			// X bounds
			double x_min = b_left ? ab : ac;
			double x_max = b_left ? ac : ab;
			assert(x_min <= x_max);
			unsigned int from_x = (unsigned int)x_min + 1;
			unsigned int to_x   = x_max;

			auto dy = (y - g_y) * dzdy;
			for (unsigned int x = from_x; x <= to_x; x++) {
				auto inv_z = inv_g_z + dy + (x - g_x) * dzdx;
				if (replace(x, y, inv_z)) {
					callback(x, y);
				}
			}
		}
	}

	// Now middle to top
	{
		unsigned int from_y = (unsigned int)b.y + 1;
		unsigned int to_y = c.y;
	
		for (unsigned int y = from_y; y <= to_y; y++) {
			double ac = f(y, a, c), bc = f(y, b, c);

			// X bounds
			double x_min = b_left ? bc : ac;
			double x_max = b_left ? ac : bc;
			assert(x_min <= x_max);
			unsigned int from_x = (unsigned int)x_min + 1;
			unsigned int to_x   = x_max;

			auto dy = (y - g_y) * dzdy;
			for (unsigned int x = from_x; x <= to_x; x++) {
				auto inv_z = inv_g_z + dy + (x - g_x) * dzdx;
				if (replace(x, y, inv_z)) {
					callback(x, y);
				}
			}
		}
	}
}

void ZBuffer::triangle(Point3D a, Point3D b, Point3D c, double d, Vector2D offset, double bias) {
	triangle(a, b, c, d, offset, bias, [](auto, auto) {});
}

void TaggedZBuffer::triangle(Point3D a, Point3D b, Point3D c, double d, Vector2D offset, IdPair pair, double bias) {
	ZBuffer::triangle(a, b, c, d, offset, bias, [this, &pair](auto x, auto y) {
		figure_ids[x + y * get_width()] = pair.figure_id;
		triangle_ids[x + y * get_width()] = pair.triangle_id;
	});
}

}
