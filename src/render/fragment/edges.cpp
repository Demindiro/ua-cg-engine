#include "render/fragment.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/matrix2d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "engine.h"
#include "lines.h"
#include "easy_image.h"
#include "render/geometry.h"
#include "render/rect.h"

/** If something looks off (vs examples), try changing these values **/

// Cursus says 1.0001, but a little lower gives better shadow quality & seems
// to be consistent with the example images
#define Z_BIAS (1.00001)
// Ditto
#define Z_SHADOW_BIAS (1.5e-6)

namespace engine {
namespace render {

using namespace std;

img::EasyImage draw(vector<LineFigure> figures, unsigned int size, Color background, bool with_z) {
	if (figures.empty()) {
		return img::EasyImage(0, 0);
	}

	// Determine bounds
	Rect rect;
	rect.min.x = rect.min.y = +numeric_limits<double>::infinity();
	rect.max.x = rect.max.y = -numeric_limits<double>::infinity();
	for (auto &f : figures) {
		for (auto &p : f.points) {
			rect |= project(p);
		}
	}

	double d;
	Vector2D offset;
	auto img = create_img(rect, size, background, d, offset);

	// Transform & draw lines
	if (with_z) {
		ZBuffer z(img.get_width(), img.get_height());
		for (auto &f : figures) {
			for (auto &e : f.edges) {
				auto a = project(f.points[e.a], d, offset);
				auto b = project(f.points[e.b], d, offset);
				if (with_z) {
					img.draw_zbuf_line(
						z,
						round_up(a.x), round_up(a.y), f.points[e.a].z,
						round_up(b.x), round_up(b.y), f.points[e.b].z,
						f.color.to_img_color()
					);
				} else {

				}
			}
		}
	} else {
		for (auto &f : figures) {
			for (auto &e : f.edges) {
				auto a = project(f.points[e.a], d, offset);
				auto b = project(f.points[e.b], d, offset);
				img.draw_line(
					round_up(a.x), round_up(a.y),
					round_up(b.x), round_up(b.y),
					f.color.to_img_color()
				);
			}
		}
	}

	return img;
}

}
}
