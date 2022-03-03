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

	static inline void lines_part(img::EasyImage &img, img::Color fg, uint n, uint ox, uint oy, uint w, uint h, bool flip_x, bool flip_y) {
		for (uint i = 0; i < n; i++) {
			// Multiply by two to represent halves (0.0, 0.5, 1.0, 1.5 ...)
			// Using only integers should be somewhat faster than converting between double <-> int
			uint x = i * (w - 1) * 2 / (n - 1);
			uint y = i * (h - 1) * 2 / (n - 1);
			// Round up & divide by 2 to get real values.
			x = ((x + 1) & ~1) / 2;
			y = ((y + 1) & ~1) / 2;
			if (flip_x != flip_y)
				y = h - 1 - y;
			Point2D a(ox + x, oy + (flip_y ? 0 : h - 1));
			Point2D b(ox + (flip_x ? w - 1 : 0), oy + y);
			Line2D(a, b, fg).draw(img);
		}
	}

	static void lines_quartercircle(img::EasyImage &img, img::Color fg, uint n) {
		lines_part(img, fg, n, 0, 0, img.get_width(), img.get_height(), false, false);
	}

	static void lines_diamond(img::EasyImage &img, img::Color fg, uint n) {
		uint x = img.get_width() / 2, y = img.get_height() / 2;
		lines_part(img, fg, n, 0, 0, x, y, true, false);
		// -1 to avoid 2px wide horizontal & vertical lines
		lines_part(img, fg, n, x - 1, y - 1, x, y, false, true);
		lines_part(img, fg, n, x - 1, 0, x, y, false, false);
		lines_part(img, fg, n, 0, y - 1, x, y, true, true);
	}

	static void lines_eye(img::EasyImage &img, img::Color fg, uint n) {
		lines_part(img, fg, n, 0, 0, img.get_width(), img.get_height(), false, false);
		lines_part(img, fg, n, 0, 0, img.get_width(), img.get_height(), true, true);
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
