#include "zbuffer.h"
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
	
	// Project
	a.x = a.x * (d / -a.z) + offset.x, a.y = a.y * (d / -a.z) + offset.y;
	b.x = b.x * (d / -b.z) + offset.x, b.y = b.y * (d / -b.z) + offset.y;
	c.x = c.x * (d / -c.z) + offset.x, c.y = c.y * (d / -c.z) + offset.y;

	// These center coordaintes must be projected.
	double g_x = (a.x + b.x + c.x) / 3;
	double g_y = (a.y + b.y + c.y) / 3;

	// Y bounds
	double y_min = min(min(a.y, b.y), c.y);
	double y_max = max(max(a.y, b.y), c.y);
	// 1.0 --> round(1.5) --> 2.0
	// 1.9 --> floor(2.9) --> 2.0
	unsigned int from_y = (unsigned int)y_min + 1;
	// 1.0 --> round(0.5) --> 1.0
	// 1.0 --> floor(1.0) --> 1.0
	unsigned int to_y = y_max;

	for (unsigned int y = from_y; y <= to_y; y++) {
		// Find intersections
		auto f = [y](Point3D p, Point3D q, double def = INFINITY) {
			return (y - p.y) * (y - q.y) <= 0
				? q.x + (p.x - q.x) * (y - q.y) / (p.y - q.y)
				: def;
		};
		auto g = [f](Point3D p, Point3D q) {
			return f(p, q, -INFINITY);
		};
		double xl = min(min(f(a, b), f(b, c)), f(c, a));
		double xr = max(max(g(a, b), g(b, c)), g(c, a));

		// X bounds
		double x_min = min(xl, xr);
		double x_max = max(xl, xr);
		unsigned int from_x = (unsigned int)x_min + 1;
		unsigned int to_x   = x_max;

		for (unsigned int x = from_x; x <= to_x; x++) {
			auto inv_z = bias * inv_g_z + (x - g_x) * dzdx + (y - g_y) * dzdy;
			if (replace(x, y, inv_z)) {
				callback(x, y);
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
