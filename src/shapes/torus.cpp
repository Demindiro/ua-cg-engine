#include "shapes/torus.h"
#include <vector>
#include "engine.h"
#include "shapes/circle.h"

using namespace std;

namespace shapes {
	static void torus(ini::Section &conf, vector<Vector3D> &points, int &n, int &m) {
		auto mr = conf["R"].as_double_or_die();
		auto sr = conf["r"].as_double_or_die();
		n = conf["n"].as_int_or_die();
		m = conf["m"].as_int_or_die();

		points = vector<Vector3D>(m * n);

		double d = 2 * M_PI / m;
		for (int i = 0; i < m; i++) {
			auto x = 0;
			auto y = mr + sin(i * d) * sr;
			auto z = cos(i * d) * sr;
			points[i] = Vector3D::point(x, y, z);
		}

		auto rot = Rotation(2 * M_PI / n).z();
		for (int i = 1; i < n; i++) {
			for (int j = 0; j < m; j++) {
				points[i * m + j] = points[(i - 1) * m + j] * rot;
			}
		}

	}

	void torus(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		vector<Vector3D> points;
		int n, m;
		torus(conf, points, n, m);
		vector<Edge> edges;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				edges.push_back({ i * m + j, i * m + (j + 1) % m });
				edges.push_back({ i * m + j, (i + 1) % n * m + j });
			}
		}
		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	void torus(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		vector<Vector3D> points;
		int n, m;
		torus(conf, points, n, m);
		vector<Face> faces;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				int k = (i + 1) % n;
				int l = (j + 1) % m;
				faces.push_back({ i * m + j, k * m + j, i * m + l });
				faces.push_back({ k * m + l, k * m + j, i * m + l });
			}
		}
		platonic(conf, mat_project, triangles, points.data(), points.size(), faces.data(), faces.size());
	}
}
