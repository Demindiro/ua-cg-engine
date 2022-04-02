#include "render/triangle.h"
#include <limits>
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/matrix2d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "lines.h"
#include "easy_image.h"
#include "render/geometry.h"
#include "render/rect.h"

using namespace std;

namespace engine {
namespace render {

Rect TriangleFigure::bounds_projected() const {
	Rect r;
	r.min.x = r.min.y = +numeric_limits<double>::infinity();
	r.max.x = r.max.y = -numeric_limits<double>::infinity();
	if (clipped) {
		// There may still be points that are now unused, so iterate over the triangles to find
		// the active points.
		for (auto &t : faces) {
			r = r | project(points[t.a]) | project(points[t.b]) | project(points[t.c]);
		}
	} else {
		for (auto &p : points) {
			r |= project(p);
		}
	}
	return r;
}

Rect ZBufferTriangleFigure::bounds_projected() const {
	Rect r;
	r.min.x = r.min.y = +numeric_limits<double>::infinity();
	r.max.x = r.max.y = -numeric_limits<double>::infinity();
	for (auto &p : points) {
		r |= project(p);
	}
	return r;
}

}
}
