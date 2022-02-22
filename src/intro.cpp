#include "intro.h"

#include <cmath>
#include <vector>

#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"

typedef unsigned int uint;

namespace intro {
	static inline img::Color tup_to_color(std::vector<double> v) {
		return img::Color(std::round(v.at(0) * 255), std::round(v.at(1) * 255), std::round(v.at(2) * 255));
	}

	// Returns -1 if lower than 0, otherwise 1.
	static inline int signum_or_one(int x) {
		return x < 0 ? -1 : 1;
	}

	void color_rectangle(img::EasyImage &img, const ini::Configuration &) {
		for (uint x = 0; x < img.get_width(); x++) {
			for (uint y = 0; y < img.get_height(); y++) {
				auto r = x * 256 / img.get_width();
				auto g = y * 256 / img.get_height();
				auto b = (r + g) % 256;
				img(x, y) = img::Color(r, g, b);
			}
		}
	}

	void blocks(img::EasyImage &img, const ini::Configuration &conf) {
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
	}

	static void line(img::EasyImage &img, uint from_x, uint from_y, uint to_x, uint to_y, img::Color color) {
		int dx = to_x - from_x, dy = to_y - from_y;
		if (std::abs(dx) < std::abs(dy)) {
			// Iterate over y
			auto s = signum_or_one(dy);
			auto f = (double)dx / dy * s;
			double fdx = from_x;
			for (unsigned int y = from_y; y != to_y; y += s) {
				img(std::round(fdx), y) = color;
				fdx += f;
			}
		} else {
			// Iterate over x
			auto s = signum_or_one(dx);
			auto f = (double)dy / dx * s;
			double fdy = from_y;
			for (unsigned int x = from_x; x != to_x; x += s) {
				img(x, std::round(fdy)) = color;
				fdy += f;
			}
		}
		img(to_x, to_y) = color;
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
			line(img, ox + x, oy + (flip_y ? 0 : h - 1), ox + (flip_x ? w - 1 : 0), oy + y, fg);
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

	void lines(img::EasyImage &img, const ini::Configuration &conf) {
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
			throw new TypeException(type);
		}
	}
}
