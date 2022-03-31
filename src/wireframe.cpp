#include "wireframe.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include "engine.h"
#include "easy_image.h"
#include "ini_configuration.h"
#include "l_system.h"
#include "l_parser.h"
#include "lines.h"
#include "math/vector3d.h"
#include "math/matrix4d.h"
#include "shapes.h"

using namespace std;
using namespace shapes;

namespace wireframe {

	void line_drawing(ini::Section &conf, Matrix4D &mat_project, vector<Line3D> &lines) {
		// Read transformation & color
		auto mat = transform_from_conf(conf, mat_project);
		auto color = color_from_conf(conf);

		// Read points & transform
		vector<Point3D> points;
		auto points_n = (unsigned int)conf["nrPoints"].as_int_or_die();
		points.reserve(points_n);
		for (unsigned int i = 0; i < points_n; i++) {
			auto p = tup_to_point3d(conf[string("point") + to_string(i)].as_double_tuple_or_die());
			points.push_back(p * mat);
		}

		// Read lines
		auto lines_n = (unsigned int)conf["nrLines"].as_int_or_die();
		lines.reserve(lines.size() + lines_n);
		for (unsigned int i = 0; i < lines_n; i++) {
			auto line = conf[string("line") + to_string(i)].as_int_tuple_or_die();
			auto b = points.at(line.at(1)), a = points.at(line.at(0));
			lines.push_back({{a.x, a.y, a.z}, {b.x, b.y, b.z}, color.to_img_color() });
		}
	}

	struct Cursor3D {
		Point3D pos;
		Matrix4D rot;
	};

	struct DrawSystem3D {
		LParser::LSystem3D sys;
		Matrix4D project;
		Rotation drot;
		Cursor3D current;
		Cursor3D *saved;
		vector<Line3D> &lines;
		mt19937 rng;
		img::Color color;

		DrawSystem3D(vector<Line3D> &lines) : lines(lines) {
			random_device rd;
			rng = mt19937(rd());
		}
	};

	static void draw_sys(DrawSystem3D &s, const string &str, int depth) {
		for (char c : str) {
			auto f = [&](auto rot) { s.current.rot = rot * s.current.rot; };
			switch (c) {
			case '+':
				f(s.drot.z());
				break;
			case '-':
				f(s.drot.inv().z());
				break;
			case '^':
				f(s.drot.inv().y());
				break;
			case '&':
				f(s.drot.y());
				break;
			case '/':
				f(s.drot.x());
				break;
			case '\\':
				f(s.drot.inv().x());
				break;
			case '(':
				*s.saved++ = s.current;
				break;
			case ')':
				s.current = *--s.saved;
				break;
			case '|':
				f(Rotation(-1, 0).z());
				break;
			default:
				if (depth > 0) {
					auto r = s.sys.get_replacement(c);
					auto sum = r.get_weights_sum();
					if (sum == 0)
						// Avoid UB as sum - 1 has to be >= 0
						continue;
					int rule = uniform_int_distribution<>(0, sum - 1)(s.rng);
					draw_sys(s, r.pick(rule), depth - 1);
				} else if (s.sys.draw(c)) {
					auto p = s.current.pos * s.project;
					s.current.pos += Vector3D(1, 0, 0) * s.current.rot;
					auto q = s.current.pos * s.project;
					s.lines.push_back({{p.x, p.y, p.z}, {q.x, q.y, q.z}, s.color});
				}
			}
		}
	}

	void l_system(ini::Section &conf, Matrix4D &mat_project, vector<Line3D> &lines) {
		DrawSystem3D s(lines);
		Cursor3D saved[2048];
		{
			ifstream f(conf["inputfile"].as_string_or_die());
			f >> s.sys;
		}
		s.project = transform_from_conf(conf, mat_project);
		s.color = color_from_conf(conf).to_img_color();
		s.current = { Point3D(), {} };
		s.drot = Rotation(s.sys.get_angle() / 180 * M_PI);
		s.saved = saved;
		draw_sys(s, s.sys.get_initiator(), s.sys.get_nr_iterations());
	}
}
