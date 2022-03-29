#include "zbuffer.h"
#include "engine.h"
#include "point3d.h"
#include "vector3d.h"

void ZBuffer::triangle(Point3D a, Point3D b, Point3D c, double d, double dx, double dy, IdPair pair, double bias) {
	using namespace std;

	// Find repricoral Z-values first, which require unprojected points
	// Optimized version of 1 / (3 * a.z) + 1 / (3 * b.z) + 1 / (3 * c.z)
	// The former will emit 3 div instructions even with -Ofast
	double inv_g_z = (b.z * c.z + a.z * c.z + a.z * b.z) / (3 * a.z * b.z * c.z);
	double dzdx, dzdy;
	{
		auto u = Vector3D::vector(b.x - a.x, b.y - a.y, b.z - a.z);
		auto v = Vector3D::vector(c.x - a.x, c.y - a.y, c.z - a.z);
		auto w = u.cross(v);
		auto dk = d * w.dot(Vector3D::point(a.x, a.y, a.z));
		dzdx = -w.x / dk;
		dzdy = -w.y / dk;
	}
	
	// Project
	a.x = a.x * d / -a.z + dx, a.y = a.y * d / -a.z + dy;
	b.x = b.x * d / -b.z + dx, b.y = b.y * d / -b.z + dy;
	c.x = c.x * d / -c.z + dx, c.y = c.y * d / -c.z + dy;

	// These center coordaintes must be projected.
	double g_x = (a.x + b.x + c.x) / 3;
	double g_y = (a.y + b.y + c.y) / 3;

	// Y bounds
	double y_min = min(min(a.y, b.y), c.y);
	double y_max = max(max(a.y, b.y), c.y);
	unsigned int from_y = round_up(y_min + 0.5);
	unsigned int to_y   = round_up(y_max - 0.5);

	for (unsigned int y = from_y; y <= to_y; y++) {
		// Find intersections
		auto f = [y](Point3D &p, Point3D &q, double def = INFINITY) {
			return (y - p.y) * (y - q.y) <= 0
				? q.x + (p.x - q.x) * (y - q.y) / (p.y - q.y)
				: def;
		};
		auto g = [f](Point3D &p, Point3D &q) {
			return f(p, q, -INFINITY);
		};
		double xl = min(min(f(a, b), f(b, c)), f(c, a));
		double xr = max(max(g(a, b), g(b, c)), g(c, a));

		// X bounds
		double x_min = min(xl, xr);
		double x_max = max(xl, xr);
		unsigned int from_x = round_up(x_min + 0.5);
		unsigned int to_x   = round_up(x_max - 0.5);

		for (unsigned int x = from_x; x <= to_x; x++) {
			pair.inv_z = bias * inv_g_z + (x - g_x) * dzdx + (y - g_y) * dzdy;
			this->set(x, y, pair);
		}
	}
}
