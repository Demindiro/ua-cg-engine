#include "shapes/wavefront.h"
#include <fstream>
#include <iterator>
#include <limits>
#include <set>
#include <string>
#include <sstream>
#include "ini_configuration.h"
#include "math/point3d.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

// References:
//
// https://raw.githubusercontent.com/Alhadis/language-wavefront/v1.0.2/docs/obj-spec.pdf
// https://www.fileformat.info/format/wavefrontobj/egff.htm
// https://www.fileformat.info/format/material/
//
// Supported ntry types. All unrecognized entries will cause an exception.
// - [ ] f (faces)
// - [ ] g (group name)
// - [ ] s (smoothing group)
// - [ ] mtllib
// - [ ] usemtl
// - [ ] v (point)
// - [ ] vt (uv)
// - [ ] vn (normal)
//
// We're parsing it directly to avoid an excessively huge amount of allocations & avoid
// unneccessary indirection in general.

void wavefront(const Configuration &conf, FaceShape &shape, Material &mat) {

	// Find all unique point/uv/normal triples & create points & faces
	struct Triple {
		int pi, ti, ni;
		bool operator <(const Triple &r) const {
			return pi != r.pi ? pi < r.pi : (ti != r.ti ? ti < r.ti : ni < r.ni);
		}
	};

	map<Triple, unsigned int> triples;
	string usemtl, mtllib;
	vector<Point3D> points;
	vector<Point2D> uvs;
	vector<Vector3D> normals;

	ifstream f(conf.section["file"].as_string_or_die());
	cout << "Reading Wavefront file" << endl;
	string line, prefix, token;
	int no_uv = -1, no_normals = -1;

	unsigned int line_i = 0;
	while (getline(f, line)) {
		line_i++;
		istringstream tok(line);
		istream_iterator<string> it(tok);
		auto end = istream_iterator<string>();
		if (it == end) {
			// Empty line.
			continue;
		}

		auto next_string = [&]() {
			++it;
			if (it == end) {
				throw WavefrontParseException(line_i, "expected token");
			}
			return *it;
		};
		auto next_double = [&]() {
			return stod(next_string());
		};
		auto maybe_next_double = [&]() {
			++it;
			if (it == end) {
				return numeric_limits<double>::signaling_NaN();
			}
			return stod(*it);
		};
		auto no_next = [&]() {
			++it;
			if (it != end) {
				throw WavefrontParseException(line_i, "too many tokens");
			}
		};

		if (*it == "v") {
			auto x = next_double();
			auto y = next_double();
			auto z = next_double();
			maybe_next_double();
			points.push_back({ x, y, z });
		} else if (*it == "vt") {
			auto u = next_double();
			auto v = maybe_next_double();
			maybe_next_double();
			uvs.push_back({ u, v });
		} else if (*it == "vn") {
			auto x = next_double();
			auto y = next_double();
			auto z = next_double();
			normals.push_back({ x, y, z });
		} else if (*it == "f") {
			auto get = [&](auto str) {
				istringstream s(str);
				Triple t = { 0, 0, 0 };
				if (!getline(s, token, '/')) {
					throw WavefrontParseException(line_i, "face vertex must have at least one point");
				}
				t.pi = stoi(token);
				if (getline(s, token, '/') && token != "") {
					t.ti = stoi(token);
				}
				if (getline(s, token, '/')) {
					t.ni = stoi(token);
				}
				if (triples.count(t) == 0) {
					triples[t] = (unsigned int)triples.size();
				}
				return triples[t];
			};
			// Get first three required triples.
			auto a = get(next_string());
			auto b = get(next_string());
			auto c = get(next_string());
			shape.faces.push_back({ a, b, c });
			// Push remaining triangles to form polygon.
			while (++it != end) {
				b = c;
				c = get(*it);
				shape.faces.push_back({ a, b, c });
			}
		} else if (*it == "s" || *it == "g") {
			// Ignore anything group-related for now.
			continue;
		} else if (*it == "usemtl") {
			auto s = next_string();
			usemtl.swap(s);
			if (!s.empty() && s != usemtl) {
				throw WavefrontParseException(line_i, "multiple different materials are not supported");
			}
		} else if (*it == "mtllib") {
			if (!mtllib.empty()) {
				throw WavefrontParseException(line_i, "only one mtllib can be loaded");
			}
			mtllib = next_string();
		} else if (it->size() > 0 && it->front() == '#') {
			continue; // Skip no_next() at the end.
		} else {
			throw WavefrontParseException(line_i, "unknown keyword '" + *it + "'");
		}
		no_next();
	}

	// Construct actual vertexes from face triples.
	shape.points.resize(triples.size());
	shape.uvs.resize(triples.size());
	shape.normals.resize(triples.size());
	for (auto [t, i] : triples) {
		assert(i < triples.size());
		shape.points[i] = points.at(t.pi - 1);
		shape.uvs[i] = t.ti > 0 ? uvs.at(t.ti - 1) : Point2D();
		shape.normals[i] = t.ni > 0 ? normals.at(t.ni - 1) : Vector3D();
	}

	// Parse material if any is provided
	if (mtllib != "" && usemtl != "") {
		ifstream f(mtllib);
		
		// Find proper material
		unsigned int line_i = 0;
		while (getline(f, line)) {
			line_i++;
			istringstream s(line);
			istream_iterator<string> it(s);
			auto end = istream_iterator<string>();
			if (it == end || *it != "newmtl") {
				continue;
			}
			goto found;
		}
		throw WavefrontParseException(0, "material " + usemtl + " not found");
	found:

		while (getline(f, line)) {
			line_i++;
			istringstream s(line);
			istream_iterator<string> it(s);
			auto end = istream_iterator<string>();
			if (it == end && *it == "newmtl") {
				break;
			}

			auto next_string = [&]() {
				++it;
				if (it == end) {
					throw WavefrontParseException(line_i, "expected token");
				}
				return *it;
			};
			auto next_double = [&]() {
				return stod(next_string());
			};
			auto maybe_next_double = [&]() {
				++it;
				if (it == end) {
					return numeric_limits<double>::signaling_NaN();
				}
				return stod(*it);
			};
			auto no_next = [&]() {
				++it;
				if (it != end) {
					throw WavefrontParseException(line_i, "too many tokens");
				}
			};

			if (*it == "Ka" || *it == "Kd" || *it == "Ks") {
				char t = (*it)[1];
				auto r = next_double();
				auto g = next_double();
				auto b = next_double();
				switch (t) {
				case 'a': mat.ambient = { r, g, b }; break;
				case 'd': mat.diffuse = { r, g, b }; break;
				case 's': mat.specular = { r, g, b }; break;
				default: UNREACHABLE;
				}
			} else if (*it == "Ns") {
				mat.reflection = next_double();
			} else if (*it == "map_Ka" || *it == "map_Kd" || *it == "map_Ks") {
				// TODO do we need to support per-light type textures? It seems
				// excessive...
				auto path = next_string();
				if (!mat.texture.has_value()) {
					img::EasyImage img;
					ifstream f(path);
					f >> img;
					mat.texture.emplace(Texture(std::move(img)));
				}
			} else {
				// TODO we shouldn't ignore other properties.
				continue;
			}
			no_next();
		}
	}

	cout << "Done parsing" << endl;
}

}
}
