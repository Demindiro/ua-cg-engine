#define _USE_MATH_DEFINES // M_PI

#include "l_system.h"
#include <cmath>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"
#include "l_parser.h"
#include "lines.h"

#define MAX_CURSOR_COUNT 2048

using namespace std;

namespace l_system {

	struct Mat2D {
		double x, y;
	};

	struct Cursor {
		double x, y, dx, dy;
	};

	struct DrawSystem2D {
		Lines2D lines;
		Cursor c { 0, 0, 0, 0 };
		Mat2D rot;
		Cursor saved[MAX_CURSOR_COUNT];
		int saved_i = 0;
		LParser::LSystem2D sys;
		Color color;
		mt19937 rng;

		DrawSystem2D() {
			random_device rd;
			rng = mt19937(rd());
		}
	};

	static Mat2D deg_to_mat2d(double a) {
		Mat2D m;
		m.x = cos(a / 180 * M_PI);
		m.y = sin(a / 180 * M_PI);
		return m;
	}

	static void mat2d_rot(Mat2D m, double &x, double &y) {
		double m00 = m.x, m01 = -m.y, m10 = m.y, m11 = m.x;
		auto nx = m00 * x + m01 * y, ny = m10 * x + m11 * y;
		x = nx, y = ny;
	}

	static void mat2d_rot_rev(Mat2D mat, double &x, double &y) {
		mat.y = -mat.y;
		mat2d_rot(mat, x, y);
	}

	static void draw_sys_2d(DrawSystem2D &s, const std::string &str, int depth) {
		for (char c : str) {
			switch (c) {
			case '+':
				mat2d_rot(s.rot, s.c.dx, s.c.dy);
				break;
			case '-':
				mat2d_rot_rev(s.rot, s.c.dx, s.c.dy);
				break;
			case '(':
				s.saved[s.saved_i++] = s.c;
				break;
			case ')':
				s.c = s.saved[--s.saved_i];
				break;
			default:
				if (depth > 0) {
					auto r = s.sys.get_replacement(c);
					auto sum = r.get_weights_sum();
					if (sum == 0)
						// Avoid UB as sum - 1 has to be >= 0
						continue;
					int rule = uniform_int_distribution<>(0, sum)(s.rng);
					draw_sys_2d(s, r.pick(rule), depth - 1);
				} else if (s.sys.draw(c)) {
					auto nx = s.c.x + s.c.dx, ny = s.c.y + s.c.dy;
					s.lines.add(Line2D(Point2D(s.c.x, s.c.y), Point2D(nx, ny), s.color));
					s.c.x = nx, s.c.y = ny;
				}
			}
		}
	}

	img::EasyImage l_2d(const ini::Configuration &conf) {
		DrawSystem2D draw_sys;

		auto bg = tup_to_color(conf["General"]["backgroundcolor"].as_double_tuple_or_die());
		auto size = conf["General"]["size"].as_int_or_die();
		auto file = conf["2DLSystem"]["inputfile"].as_string_or_die();
		draw_sys.color = tup_to_color(conf["2DLSystem"]["color"].as_double_tuple_or_die());

		{
			std::ifstream f(file);
			f >> draw_sys.sys;
		}

		auto d = deg_to_mat2d(draw_sys.sys.get_starting_angle());
		draw_sys.c.dx = d.x;
		draw_sys.c.dy = d.y;
		draw_sys.rot = deg_to_mat2d(draw_sys.sys.get_angle());

		if (draw_sys.sys.get_nr_iterations() > MAX_CURSOR_COUNT) {
			throw out_of_range("Iterations");
		}
		draw_sys_2d(draw_sys, draw_sys.sys.get_initiator(), draw_sys.sys.get_nr_iterations());

		return draw_sys.lines.draw(size, bg);
	}
}
