#include "shapes/wavefront.h"
#include <fstream>
#include "ini_configuration.h"
#include "obj_parser.h"
#include "math/point3d.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

void wavefront(const ini::Section &conf, FaceShape &shape) {
	ifstream f(conf["file"].as_string_or_die());
	obj::ObjectGroup obj(f);
	for (const auto &p : obj.get_vertexes()) {
		shape.points.push_back(tup_to_point3d(p));
	}
	for (const auto &p : obj.get_polygons()) {
		const auto &ind = p.get_indexes();
		for (size_t i = 2; i < ind.size(); i++) {
			shape.faces.push_back({ ind[0] - 1, ind[i - 1] - 1, ind[i] - 1 });
		}
	}
}

}
}
