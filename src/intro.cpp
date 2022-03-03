#include "intro.h"

#include <cmath>
#include <vector>

#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"
#include "lines.h"

typedef unsigned int uint;

namespace intro {

	static img::EasyImage img_from_conf(const ini::Configuration &conf) {
		auto width  = conf["ImageProperties"]["width" ].as_int_or_die();
		auto height = conf["ImageProperties"]["height"].as_int_or_die();
		return img::EasyImage(width, height);
	}

	img::EasyImage color_rectangle(const ini::Configuration &conf) {
		auto img = img_from_conf(conf);

		for (uint x = 0; x < img.get_width(); x++) {
			for (uint y = 0; y < img.get_height(); y++) {
				auto r = x * 256 / img.get_width();
				auto g = y * 256 / img.get_height();
				auto b = (r + g) % 256;
				img(x, y) = img::Color(r, g, b);
			}
		}

		return img;
	}

	img::EasyImage blocks(const ini::Configuration &conf) {
		auto img = img_from_conf(conf);

		auto props = conf["BlockProperties"];
		auto color_a = tup_to_color(props["colorWhite"].as_double_tuple_or_die());
		auto color_b = tup_to_color(props["colorBlack"].as_double_tuple_or_die());
		uint n_x = props["nrXBlocks"].as_int_or_die();
		uint n_y = props["nrYBlocks"].as_int_or_die();

		if (props["invertColors"].as_bool_or_default(false)) {
			auto t = color_a;
			color_a = color_b, color_b = t;
		}

		for (uint x = 0; x < img.get_width(); x++) {
			for (uint y = 0; y < img.get_height(); y++) {
				auto bx = x * n_x / img.get_width(), by = y * n_y / img.get_height();
				img(x, y) = (bx + by) % 2 == 0 ? color_a : color_b;
			}
		}

		return img;
	}

	static inline void lines_part(img::EasyImage &img, img::Color fg, uint n, double ox, double oy, double dx, double dy) {
		dx /= n - 1;
		dy /= n - 1;
		for (uint i = 0; i < n; i++) {
			auto x = ox + dx * i;
			auto y = oy + dy * (n - 1 - i);
			Line2D(Point2D(x, oy), Point2D(ox, y), fg).draw(img);
		}
	}

	static void lines_quartercircle(img::EasyImage &img, img::Color fg, uint n) {
		double x = img.get_width() - 1;
		double y = img.get_height() - 1;
		lines_part(img, fg, n, 0, y, x, -y);
	}

	static void lines_diamond(img::EasyImage &img, img::Color fg, uint n) {
		double x = (img.get_width() - 1) / 2.0;
		double y = (img.get_height() - 1) / 2.0;
		lines_part(img, fg, n, x, y, x, y);
		lines_part(img, fg, n, x, y, -x, y);
		lines_part(img, fg, n, x, y, x, -y);
		lines_part(img, fg, n, x, y, -x, -y);
	}

	static void lines_eye(img::EasyImage &img, img::Color fg, uint n) {
		double x = img.get_width() - 1;
		double y = img.get_height() - 1;
		lines_part(img, fg, n, 0, y, x, -y);
		lines_part(img, fg, n, x, 0, -x, y);
	}

	img::EasyImage lines(const ini::Configuration &conf) {
		auto img = img_from_conf(conf);

		auto props = conf["LineProperties"];
		auto type = props["figure"].as_string_or_die();
		auto bg = tup_to_color(props["backgroundcolor"].as_double_tuple_or_die());
		auto fg = tup_to_color(props["lineColor"].as_double_tuple_or_die());
		auto n = props["nrLines"].as_int_or_die();

		img.clear(bg);

		if (type == "QuarterCircle") {
			lines_quartercircle(img, fg, n);
		} else if (type == "Diamond") {
			lines_diamond(img, fg, n);
		} else if (type == "Eye") {
			lines_eye(img, fg, n);
		} else {
			throw TypeException(type);
		}

		return img;
	}
}
