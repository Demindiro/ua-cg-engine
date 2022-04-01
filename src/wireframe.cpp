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

namespace engine {
namespace wireframe {

using namespace std;
using namespace shapes;

void line_drawing(const ini::Section &conf, EdgeShape &f) {
	// Read points & transform
	auto points_n = (unsigned int)conf["nrPoints"].as_int_or_die();
	f.points.reserve(points_n);
	for (unsigned int i = 0; i < points_n; i++) {
		auto p = tup_to_point3d(conf[string("point") + to_string(i)].as_double_tuple_or_die());
		f.points.push_back(p);
	}

	// Read lines
	auto lines_n = (unsigned int)conf["nrLines"].as_int_or_die();
	f.edges.reserve(lines_n);
	for (unsigned int i = 0; i < lines_n; i++) {
		auto line = conf[string("line") + to_string(i)].as_int_tuple_or_die();
		f.edges.push_back({ (unsigned int)line.at(1), (unsigned int)line.at(0) });
	}
}

struct Cursor3D {
	Matrix4D rot;
	Point3D pos;
	unsigned int i = 0;
};

struct DrawSystem3D {
	LParser::LSystem3D sys;
	Rotation drot;
	Cursor3D current;
	Cursor3D *saved;
	EdgeShape &shape;
	mt19937 rng;

	DrawSystem3D(EdgeShape &shape) : shape(shape) {
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
				s.current.pos += Vector3D(1, 0, 0) * s.current.rot;
				auto i = (unsigned int)s.shape.points.size();
				s.shape.points.push_back(s.current.pos);
				s.shape.edges.push_back({ s.current.i, i });
				s.current.i = i;
			}
		}
	}
}

void l_system(const ini::Section &conf, EdgeShape &f) {
	DrawSystem3D s(f);
	Cursor3D saved[2048];
	{
		ifstream f(conf["inputfile"].as_string_or_die());
		f >> s.sys;
	}
	f.points.push_back({});
	s.drot = Rotation(s.sys.get_angle() / 180 * M_PI);
	s.saved = saved;
	draw_sys(s, s.sys.get_initiator(), s.sys.get_nr_iterations());
}

}
}
