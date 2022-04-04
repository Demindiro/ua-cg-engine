#include "shapes/torus.h"
#include <cassert>
#include <vector>
#include "engine.h"
#include "shapes/circle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

static void torus(
	const Configuration &conf,
	vector<Point3D> &points,
	vector<Vector3D> *normals,
	unsigned int &n,
	unsigned int &m,
	bool point_normals
) {
	auto mr = conf.section["R"].as_double_or_die();
	auto sr = conf.section["r"].as_double_or_die();
	n = conf.section["n"].as_int_or_die();
	m = conf.section["m"].as_int_or_die();
	assert(n > 0 && "torus with no points");
	assert(m > 0 && "torus with no points");

	points.resize(m * n);
	if (normals != nullptr) {
		normals->resize(m * n);
	}

	{
		Rotation d(-2 * M_PI / m), r;
		for (unsigned int i = 0; i < m; i++) {
			points[i] = { 0, 0, sr };
			points[i] *= r.x();
			if (normals != nullptr && point_normals) {
				(*normals)[i] = points[i].to_vector();
			}
			points[i].y += mr;
			r *= d;
		}
	}

	auto rot = Rotation(2 * M_PI / n).z();
	for (unsigned int i = 1; i < n; i++) {
		for (unsigned int j = 0; j < m; j++) {
			points[i * m + j] = points[(i - 1) * m + j] * rot;
			if (normals != nullptr && point_normals) {
				(*normals)[i * m + j] = (*normals)[(i - 1) * m + j] * rot;
			}
		}
	}

}

void torus(const Configuration &conf, EdgeShape &f) {
	unsigned int n, m;
	torus(conf, f.points, nullptr, n, m, false);
	for (unsigned int i = 0; i < n; i++) {
		for (unsigned int j = 0; j < m; j++) {
			f.edges.push_back({ i * m + j, i * m + (j + 1) % m });
			f.edges.push_back({ i * m + j, (i + 1) % n * m + j });
		}
	}
}

void torus(const Configuration &conf, FaceShape &f) {
	unsigned int n, m;
	torus(conf, f.points, conf.point_normals ? &f.normals : nullptr, n, m, conf.point_normals);
	for (unsigned int i = 0; i < n; i++) {
		for (unsigned int j = 0; j < m; j++) {
			unsigned int k = (i + 1) % n;
			unsigned int l = (j + 1) % m;
			f.faces.push_back({ i * m + j, i * m + l, k * m + j });
			f.faces.push_back({ k * m + l, k * m + j, i * m + l });
		}
	}
}

}
}
