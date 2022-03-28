#include "shapes/torus.h"
#include <vector>
#include "engine.h"
#include "shapes/circle.h"

using namespace std;

namespace shapes {
	static void torus(ini::Section &conf, vector<Point3D> &points, unsigned int &n, unsigned int &m) {
		auto mr = conf["R"].as_double_or_die();
		auto sr = conf["r"].as_double_or_die();
		n = conf["n"].as_int_or_die();
		m = conf["m"].as_int_or_die();

		points = vector<Point3D>(m * n);

		double d = 2 * M_PI / m;
		for (unsigned int i = 0; i < m; i++) {
			auto x = 0.0;
			auto y = mr + sin(i * d) * sr;
			auto z = cos(i * d) * sr;
			points[i] = { x, y, z };
		}

		auto rot = Rotation(2 * M_PI / n).z();
		for (unsigned int i = 1; i < n; i++) {
			for (unsigned int j = 0; j < m; j++) {
				points[i * m + j] = points[(i - 1) * m + j] * rot;
			}
		}

	}

	void torus(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points;
		unsigned int n, m;
		torus(conf.section, points, n, m);
		vector<Edge> edges;
		for (unsigned int i = 0; i < n; i++) {
			for (unsigned int j = 0; j < m; j++) {
				edges.push_back({ i * m + j, i * m + (j + 1) % m });
				edges.push_back({ i * m + j, (i + 1) % n * m + j });
			}
		}
		platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	TriangleFigure torus(const FigureConfiguration &conf) {
		vector<Point3D> points;
		unsigned int n, m;
		torus(conf.section, points, n, m);
		vector<Face> faces;
		for (unsigned int i = 0; i < n; i++) {
			for (unsigned int j = 0; j < m; j++) {
				unsigned int k = (i + 1) % n;
				unsigned int l = (j + 1) % m;
				faces.push_back({ i * m + j, k * m + j, i * m + l });
				faces.push_back({ k * m + l, k * m + j, i * m + l });
			}
		}
		return platonic(conf, points, faces);
	}
}
