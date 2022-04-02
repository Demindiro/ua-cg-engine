#include "shapes/torus.h"
#include <cassert>
#include <vector>
#include "engine.h"
#include "shapes/circle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

static void torus(const ini::Section &conf, vector<Point3D> &points, unsigned int &n, unsigned int &m) {
	auto mr = conf["R"].as_double_or_die();
	auto sr = conf["r"].as_double_or_die();
	n = conf["n"].as_int_or_die();
	m = conf["m"].as_int_or_die();
	assert(n > 0 && "torus with no points");
	assert(m > 0 && "torus with no points");

	points = vector<Point3D>(m * n);

	{
		Rotation d(-2 * M_PI / m), r;
		for (unsigned int i = 0; i < m; i++) {
			points[i] = { 0, 0, sr };
			points[i] *= r.x();
			points[i].y += mr;
			r *= d;
		}
	}

	auto rot = Rotation(2 * M_PI / n).z();
	for (unsigned int i = 1; i < n; i++) {
		for (unsigned int j = 0; j < m; j++) {
			points[i * m + j] = points[(i - 1) * m + j] * rot;
		}
	}

}

void torus(const ini::Section &conf, EdgeShape &f) {
	unsigned int n, m;
	torus(conf, f.points, n, m);
	for (unsigned int i = 0; i < n; i++) {
		for (unsigned int j = 0; j < m; j++) {
			f.edges.push_back({ i * m + j, i * m + (j + 1) % m });
			f.edges.push_back({ i * m + j, (i + 1) % n * m + j });
		}
	}
}

void torus(const ini::Section &conf, FaceShape &f) {
	unsigned int n, m;
	torus(conf, f.points, n, m);
	auto &f_faces = *f.faces;
	for (unsigned int i = 0; i < n; i++) {
		for (unsigned int j = 0; j < m; j++) {
			unsigned int k = (i + 1) % n;
			unsigned int l = (j + 1) % m;
			f_faces.push_back({ i * m + j, i * m + l, k * m + j });
			f_faces.push_back({ k * m + l, k * m + j, i * m + l });
		}
	}
}

}
}
