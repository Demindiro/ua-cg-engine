#include "wireframe.h"
#include <cmath>
#include <string>
#include <vector>
#include "engine.h"
#include "easy_image.h"
#include "ini_configuration.h"
#include "l_system.h"
#include "lines.h"
#include "vector3d.h"

using namespace std;

namespace wireframe {

	static void line_drawing(ini::Section &conf, Matrix &mat_project, vector<Line2D> &lines) {
		// Read transformation & color
		auto rot_x = conf["rotateX"].as_double_or_die() * M_PI / 180;
		auto rot_y = conf["rotateY"].as_double_or_die() * M_PI / 180;
		auto rot_z = conf["rotateZ"].as_double_or_die() * M_PI / 180;
		auto center = tup_to_point3d(conf["center"].as_double_tuple_or_die());
		auto scale = conf["scale"].as_double_or_die();
		auto color = tup_to_color(conf["color"].as_double_tuple_or_die());

		// Create transformation matrix
		// Order of operations: scale > rot_x > rot_y > rot_z > translate > project
		// NB the default constructor creates identity matrices (diagonal is 1)
		Matrix mat_scale, mat_rot_x, mat_rot_y, mat_rot_z, mat_translate;

		mat_scale(1, 1) = mat_scale(2, 2) = mat_scale(3, 3) = scale;

		mat_rot_x(2, 2) = mat_rot_x(3, 3) = cos(rot_x);
		mat_rot_x(2, 3) = sin(rot_x);
		mat_rot_x(3, 2) = -mat_rot_x(2, 3);

		mat_rot_y(1, 1) = mat_rot_y(3, 3) = cos(rot_y);
		mat_rot_y(1, 3) = -sin(rot_y);
		mat_rot_y(3, 1) = -mat_rot_y(1, 3);

		mat_rot_z(1, 1) = mat_rot_z(2, 2) = cos(rot_z);
		mat_rot_z(1, 2) = sin(rot_z);
		mat_rot_z(2, 1) = -mat_rot_z(1, 2);

		mat_translate(4, 1) = center.x;
		mat_translate(4, 2) = center.y;
		mat_translate(4, 3) = center.z;

		auto mat = mat_scale * mat_rot_x * mat_rot_y * mat_rot_z * mat_translate * mat_project;

		// Read points & transform
		vector<Vector3D> points;
		auto points_n = conf["nrPoints"].as_int_or_die();
		points.reserve(points_n);
		for (int i = 0; i < points_n; i++) {
			auto p = tup_to_point3d(conf[string("point") + to_string(i)].as_double_tuple_or_die());
			points.push_back(p * mat);
		}

		// Read lines
		auto lines_n = conf["nrLines"].as_int_or_die();
		lines.reserve(lines.size() + lines_n);
		for (int i = 0; i < lines_n; i++) {
			auto line = conf[string("line") + to_string(i)].as_int_tuple_or_die();
			auto b = points.at(line.at(1)), a = points.at(line.at(0));
			lines.push_back({{a.x, a.y}, {b.x, b.y}, color});
		}
	}

	img::EasyImage wireframe(const ini::Configuration &conf) {
		auto bg = tup_to_color(conf["General"]["backgroundcolor"].as_double_tuple_or_die());
		auto size = conf["General"]["size"].as_int_or_die();
		auto eye = tup_to_point3d(conf["General"]["eye"].as_double_tuple_or_die());
		auto nr_fig = conf["General"]["nrFigures"].as_int_or_die();

		// Create rojection matrix
		Matrix mat_eye;
		{
			auto r = eye.length();
			auto theta = atan2(eye.y, eye.x);
			auto phi = acos(eye.z / r);

			auto theta_sin = sin(theta);
			auto theta_cos = cos(theta);

			auto phi_sin = sin(phi);
			auto phi_cos = cos(phi);

			mat_eye(1, 1) = -theta_sin;
			mat_eye(1, 2) = -theta_cos * phi_cos;
			mat_eye(1, 3) = theta_cos * phi_sin;

			mat_eye(2, 1) = theta_cos;
			mat_eye(2, 2) = -theta_sin * phi_cos;
			mat_eye(2, 3) = theta_sin * phi_sin;

			mat_eye(3, 2) = phi_sin;
			mat_eye(3, 3) = phi_cos;

			mat_eye(4, 3) = -r;
		}

		// Parse figures
		vector<Line2D> lines;
		for (int i = 0; i < nr_fig; i++) {
			auto fig = conf[string("Figure") + to_string(i)];
			auto type = fig["type"].as_string_or_die();
			if (type == "LineDrawing") {
				line_drawing(fig, mat_eye, lines);
			} else {
				throw TypeException(type);
			}
		}

		// Draw
		return Lines2D(lines).draw(size, bg);
	}
}
