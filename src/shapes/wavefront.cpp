#include "shapes/wavefront.h"
#include <fstream>
#include <iterator>
#include <limits>
#include <set>
#include <string>
#include <sstream>
#include <unordered_map>
#include "ini_configuration.h"
#include "math/point3d.h"
#include "shapes.h"
#include "render/geometry.h"
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

void wavefront(const std::string &path, FaceShape &shape, Material &mat, bool &point_normals) {

	// Find all unique point/uv/normal triples
	struct Triple {
		int pi, ti, ni;
	};
	struct TripleHash {
		size_t operator ()(const Triple &t) const {
			// This is the fastest hash function I could come up with, I swear.
			return t.pi;
		}
	};
	struct TripleEqual {
		bool operator ()(const Triple &l, const Triple &r) const {
			return l.pi == r.pi && l.ti == r.ti && l.ni == r.ni;
		}
	};

	unordered_map<Triple, unsigned int, TripleHash, TripleEqual> triples;
	string usemtl, mtllib;
	vector<Point3D> points;
	vector<Point2D> uvs;
	vector<Vector3D> normals;

	ifstream f;
	char buffer[1 << 16];
	f.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
	f.open(path);
	cout << "Reading Wavefront file" << endl;
	// Try to reuse buffers as much as possible.
	// Merely moving tok & vert from inside the loops to here
	// reduced runtime on Lucy from 100s to 65s!
	string line, prefix, token;
	istringstream tok, vert;
	vector<pair<Point3D, unsigned int>> polygon;

	unsigned int line_i = 0;
	while (getline(f, line)) {
		line_i++;
		tok.clear();
		tok.seekg(0, ios::beg);
		tok.str(line);
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
			// Use two-ears theorem so we triangulate concave polygons properly.
			polygon.clear();
			while (++it != end) {
				vert.clear();
				vert.seekg(0, ios::beg);
				vert.str(*it);
				Triple t = { 0, 0, 0 };
				if (!getline(vert, token, '/')) {
					throw WavefrontParseException(line_i, "face vertex must have at least one point");
				}
				t.pi = stoi(token);
				if (getline(vert, token, '/') && token != "") {
					t.ti = stoi(token);
				}
				if (getline(vert, token, '/')) {
					t.ni = stoi(token);
				}
				if (triples.count(t) == 0) {
					triples[t] = (unsigned int)triples.size();
				}
				// TODO defining faces before vertices may be technically valid, in which case
				// we'll have to defer triangulation.
				polygon.push_back({ points.at(t.pi - 1), triples[t] });
			}
			if (polygon.size() < 3) {
				throw WavefrontParseException(line_i, "face must have at least 3 points");
			} else {
				// A vertex is an ear if the diagonal between it's two neighbours lies entirely
				// in the polygon, i.e. none of its points lie in another triangle.
				
				// Calculate the normal of the polygon.
				// This is called Newell's method. I have no idea *why* it works and I seem to be
				// unable to find a proper explanation.
				// Ref: https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal#Newell.27s_Method
				Vector3D poly_norm;
				for (size_t i = 0; i < polygon.size(); i++) {
					auto c = polygon[i].first.to_vector();
					auto n = polygon[(i + 1) % polygon.size()].first.to_vector();
					auto d = c - n, s = c + n;
					poly_norm += Vector3D(d.y * s.z, d.z * s.x, d.x * s.y);
				}

				size_t i = 0;
				while (polygon.size() > 3) {
					auto get = [&](auto n) {
						return polygon[n % polygon.size()];
					};
					// Take any point and *ignore* its neighbours.
					auto a = get(i);
					auto b = get(i + 1);
					auto c = get(i + 2);
					if ((b.first - a.first).cross(c.first - a.first).dot(poly_norm) < 0) {
						goto skip;
					}
					// Check if any other point is inside the triangle.
					for (size_t k = 0; k < polygon.size(); k++) {
						if (k != i && k != (i + 1) % polygon.size() && k != (i + 2) % polygon.size()) {
							auto pq = calc_pq(a.first, b.first, c.first, get(k).first);
							if ((0 <= pq.x && pq.x <= 1) && (0 <= pq.y && pq.y <= 1)) {
								goto skip;
							}
						}
					}
					// No other point lies in the triangle and it faces in the right direction.
					shape.faces.push_back({ a.second, b.second, c.second });
					polygon.erase(polygon.begin() + (i + 1) % polygon.size());
				skip:
					if (++i >= polygon.size()) {
						i = 0;
					}
				}
			}
			// Push the final triangle.
			shape.faces.push_back({ polygon[0].second, polygon[1].second, polygon[2].second });
		} else if (*it == "p") {
			// We don't support points so just skip.
			continue;
		} else if (*it == "l") {
			// Ditto but lines
			continue;
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
	if (triples.empty()) {
		return;
	}
	bool has_uv = triples.begin()->first.ti != 0, has_normals = triples.begin()->first.ni != 0;
	shape.points.resize(triples.size());
	shape.uvs.resize(has_uv ? triples.size() : 0);
	shape.normals.resize(has_normals ? triples.size() : 0);
	for (auto [t, i] : triples) {
		assert(i < triples.size());
		assert(t.pi != 0);
		shape.points[i] = points.at(t.pi > 0 ? t.pi - 1 : points.size() + t.pi);
		if (has_uv) {
			assert(t.ti != 0);
			shape.uvs[i] = uvs.at(t.ti > 0 ? t.ti - 1 : uvs.size() + t.ti);
		}
		if (has_normals) {
			assert(t.ni != 0);
			shape.normals[i] = normals.at(t.ni > 0 ? t.ni - 1 : normals.size() + t.ni);
		}
	}
	point_normals = has_normals;

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

void wavefront(const Configuration &conf, FaceShape &shape, Material &mat, bool &point_normals) {
	wavefront(conf.section["file"].as_string_or_die(), shape, mat, point_normals);
}

}
}
