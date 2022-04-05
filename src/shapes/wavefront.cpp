#include "shapes/wavefront.h"
#include <fstream>
#include <set>
#include "ini_configuration.h"
#include "obj_parser.h"
#include "math/point3d.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

void wavefront(const Configuration &conf, FaceShape &shape) {
	ifstream f(conf.section["file"].as_string_or_die());
	cout << "Reading Wavefront file" << endl;
	obj::ObjectGroup obj(f);
	cout << "Parsing loaded data" << endl;

	// Find all unique point/uv/normal triples & create points & faces
	struct Triple {
		int pi, ti, ni;
		bool operator <(const Triple &r) const {
			return pi != r.pi ? pi < r.pi : (ti != r.ti ? ti < r.ti : ni < r.ni);
		}
	};
	map<Triple, unsigned int> triples;
	for (const auto &p : obj.get_polygons()) {
		auto get = [&](auto i) {
			auto pi = p.get_indexes().at(i);
			auto ti = p.has_texture_indexes() ? p.get_texture_indexes().at(i) : 0;
			auto ni = p.has_normal_indexes() ? p.get_normal_indexes().at(i) : 0;
			return Triple { pi - 1, ti - 1, ni - 1 };
		};
		for (size_t i = 0; i < p.get_indexes().size(); i++) {
			auto t = get(i);
			if (triples.count(t) == 0) {
				triples[t] = (unsigned int)triples.size();
				shape.points.push_back(tup_to_point3d(obj.get_vertexes().at(t.pi)));
				/*
				shape.uv.push_back(t.ti != -1
					? tup_to_point2d(obj.get_texture_coordinates().at(t.ti))
					: Point2D()
				);
				*/
				shape.normals.push_back(t.ni != -1
					? tup_to_vector3d(obj.get_vertex_normals().at(t.ni))
					: Vector3D()
				);
			}
			assert(triples.size() == shape.points.size());
		}
		for (size_t i = 2; i < p.get_indexes().size(); i++) {
			shape.faces.push_back({ triples[get(0)], triples[get(i - 1)], triples[get(i)] });
		}
	}

	cout << "Done parsing" << endl;
}

}
}
