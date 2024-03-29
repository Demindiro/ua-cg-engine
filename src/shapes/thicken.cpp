#include "shapes/thicken.h"
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/cylinder.h"
#include "shapes/sphere.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

static void sphere(unsigned int n, EdgeShape &shape, bool) {
	sphere(n, shape);
}

static void cylinder_sides(unsigned int n, double height, EdgeShape &shape, bool) {
	cylinder_sides(n, height, shape);
}

template<class T>
static void init(const Configuration &conf, T &sphere_o, T &cylinder_o) {
	auto radius = conf.section["radius"].as_double_or_die();
	auto n = (unsigned int)conf.section["n"].as_int_or_die();
	auto m = (unsigned int)conf.section["m"].as_int_or_die();

	sphere(m, sphere_o, conf.point_normals);
	for (auto &p : sphere_o.points) {
		p = Point3D(p.to_vector() * radius);
	}

	cylinder_sides(n, 1, cylinder_o, conf.point_normals);
	for (auto &p : cylinder_o.points) {
		p.x *= radius;
		p.y *= radius;
	}
}

template<class T, class U, typename F>
static void place_cylinder(const ShapeTemplateAny &shape, Edge e, const T &cylinder, U &f, F g) {
	auto a = shape.points[e.a];
	auto b = shape.points[e.b];
	auto d = b - a;
	auto d_xy = Vector2D(d.x, d.y);
	auto l = d.length();
	Rotation r_z({ -1, 0 }, { d.x, d.y });
	Rotation r_y({ 0, 1 }, { d_xy.length(), d.z });
	auto r = r_y.y() * r_z.z();
	for (const auto &q : cylinder.points) {
		auto v = q.to_vector();
		v.z *= l;
		f.points.push_back(a + v * r);
	}
	g(r);
}

void thicken(
	const Configuration &conf,
	const ShapeTemplateAny &shape,
	EdgeShape &f
) {
	EdgeShape sphere, cylinder;
	init(conf, sphere, cylinder);
	f.points.reserve(shape.points_size * sphere.points.size() + shape.edges_size * cylinder.points.size());
	f.edges.reserve(shape.points_size * sphere.edges.size() + shape.edges_size * cylinder.edges.size());

	for (size_t ip = 0; ip < shape.points_size; ip++) {
		const auto &p = shape.points[ip];
		auto o = (unsigned int)f.points.size();
		for (const auto &e : sphere.edges) {
			f.edges.push_back({ e.a + o, e.b + o });
		}
		for (const auto &q : sphere.points) {
			f.points.push_back(p + q.to_vector());
		}
	}

	for (size_t ie = 0; ie < shape.edges_size; ie++) {
		const auto &e = shape.edges[ie];
		auto o = (unsigned int)f.points.size();
		for (const auto &g : cylinder.edges) {
			f.edges.push_back({ g.a + o, g.b + o });
		}
		place_cylinder(shape, e, cylinder, f, [](auto) {});
	}
}

void thicken(
	const Configuration &conf,
	const ShapeTemplateAny &shape,
	FaceShape &f
) {
	FaceShape sphere, cylinder;
	init(conf, sphere, cylinder);
	f.points.reserve(shape.points_size * sphere.points.size() + shape.edges_size * cylinder.points.size());
	f.faces.reserve(shape.points_size * sphere.faces.size() + shape.edges_size * cylinder.faces.size());

	for (size_t ip = 0; ip < shape.points_size; ip++) {
		const auto &p = shape.points[ip];
		auto o = (unsigned int)f.points.size();
		for (const auto &e : sphere.faces) {
			f.faces.push_back({ e.a + o, e.b + o, e.c + o });
		}
		for (const auto &q : sphere.points) {
			f.points.push_back(p + q.to_vector());
		}
		for (const auto &n : sphere.normals) {
			f.normals.push_back(n);
		}
	}

	for (size_t ie = 0; ie < shape.edges_size; ie++) {
		const auto &e = shape.edges[ie];
		auto o = (unsigned int)f.points.size();
		for (const auto &g : cylinder.faces) {
			f.faces.push_back({ g.a + o, g.b + o, g.c + o });
		}
		place_cylinder(shape, e, cylinder, f, [&](const auto &mat) {
			for (const auto &n : cylinder.normals) {
				f.normals.push_back(n * mat);
			}
		});
	}
}

}
}
