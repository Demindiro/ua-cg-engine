#include "shapes.h"
#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <vector>
#include "engine.h"
#include "ini_configuration.h"
#include "math/matrix2d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "render/aabb.h"
#include "render/fragment.h"
#include "render/geometry.h"
#include "render/light.h"
#include "shapes/buckyball.h"
#include "shapes/circle.h"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/dodecahedron.h"
#include "shapes/fractal.h"
#include "shapes/icosahedron.h"
#include "shapes/mengersponge.h"
#include "shapes/tetrahedron.h"
#include "shapes/octahedron.h"
#include "shapes/sphere.h"
#include "shapes/thicken.h"
#include "shapes/torus.h"
#include "shapes/wavefront.h"
#include "wireframe.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

static Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection, double &scale) {
	auto rot_x = conf["rotateX"].as_double_or_die() * M_PI / 180;
	auto rot_y = conf["rotateY"].as_double_or_die() * M_PI / 180;
	auto rot_z = conf["rotateZ"].as_double_or_die() * M_PI / 180;
	auto center = tup_to_point3d(conf["center"].as_double_tuple_or_die());
	scale = conf["scale"].as_double_or_die();

	// Create transformation matrix
	// Order of operations: scale > rot_x > rot_y > rot_z > translate > project
	// NB the default constructor creates identity matrices (diagonal is 1)
	Matrix4D mat_rot_x, mat_rot_y, mat_rot_z, mat_translate;

	mat_rot_x = Rotation(rot_x).x();
	mat_rot_y = Rotation(rot_y).y();
	mat_rot_z = Rotation(rot_z).z();

	mat_translate(4, 1) = center.x;
	mat_translate(4, 2) = center.y;
	mat_translate(4, 3) = center.z;

	return mat_rot_x * mat_rot_y * mat_rot_z * mat_translate * projection;
}

Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection) {
	Matrix4D mat_scale, mat;
	mat = transform_from_conf(conf, projection, mat_scale(3, 3));
	mat_scale(1, 1) = mat_scale(2, 2) = mat_scale(3, 3);
	return mat_scale * mat;
}

static Color color_from_conf(const vector<double> &c) {
	double b = c.at(2);
	double g = c.at(1);
	double r = c.at(0);
	return { r, g, b };
}

static Color try_color_from_conf(const vector<double> &c, Color def = {}) {
	if (c.size() < 3)
		return def;
	double b = c.at(2);
	double g = c.at(1);
	double r = c.at(0);
	return { r, g, b };
}

static Color color_from_conf(const ini::Entry &e) {
	return color_from_conf(e.as_double_tuple_or_die());
}

static Color try_color_from_conf(const ini::Entry &e, Color def = {}) {
	vector<double> c;
	return e.as_double_tuple_if_exists(c) ? color_from_conf(c) : def;
}

Color color_from_conf(const ini::Section &conf) {
	return color_from_conf(conf["color"]);
}

vector<Vector3D> calculate_face_normals(const vector<Point3D> &points, const vector<Face> &faces) {
	vector<Vector3D> normals;
	normals.reserve(faces.size());
	for (auto &f : faces) {
		assert(f.a < points.size());
		assert(f.b < points.size());
		assert(f.c < points.size());
		auto a = points[f.a];
		auto b = points[f.b];
		auto c = points[f.c];
		normals.push_back((b - a).cross(c - a).normalize());
	}
	return normals;
}

TriangleFigure convert(
	const FaceShape &shape,
	const Material &mat,
	const Matrix4D &transform,
	double scale,
	bool with_cubemap,
	bool with_point_normals
) {
	TriangleFigure fig;
	fig.points = shape.points;
	fig.faces = shape.faces;
	fig.ambient = mat.ambient;
	fig.diffuse = mat.diffuse;
	fig.specular = mat.specular;
	fig.reflection = mat.reflection;
	// integer powers are faster, so try to use that.
	fig.reflection_int = fig.reflection;
	if (fig.reflection_int != fig.reflection) {
		fig.reflection_int = numeric_limits<unsigned int>::max();
	}
	fig.flags.can_cull(true); // All platonics are solid (& other generated meshes are too)
	fig.flags.clipped(false);

	if (mat.texture.has_value()) {
		fig.texture = mat.texture;
		fig.uv = shape.uvs;
		assert(!fig.uv.empty() && "shape has texture but no UVs");
	}

	fig.flags.cubemap(with_cubemap);
	fig.flags.separate_normals(with_point_normals);
	fig.normals = shape.normals;
	assert(!fig.normals.empty() && "shape has no normals");

	for (auto &p : fig.normals) {
		p *= transform;
	}

	Matrix4D mat_scale, m;
	mat_scale(1, 1) = mat_scale(2, 2) = mat_scale(3, 3) = scale;
	m = mat_scale * transform;

	for (auto &p : fig.points) {
		p *= m;
	}

	return fig;
}

static TriangleFigure convert(FaceShape &shape, Material &mat, const Configuration &conf, bool with_lighting, const Matrix4D &eye) {
	const auto &section = conf.section;
	if (with_lighting) {
		mat.ambient = try_color_from_conf(section["ambientReflection"], mat.ambient);
		mat.diffuse = try_color_from_conf(section["diffuseReflection"], mat.diffuse);
		mat.specular = try_color_from_conf(section["specularReflection"], mat.specular);
		mat.reflection = section["reflectionCoefficient"].as_double_or_default(mat.reflection);
	} else {
		mat.ambient = color_from_conf(section);
	}

	// Load texture, if any
	string tex_path;
	if (section["texture"].as_string_if_exists(tex_path)) {
		{
			ifstream f(tex_path);
			img::EasyImage img;
			f >> img;
			Texture tex(std::move(img));
			mat.texture.emplace(std::move(tex));
		}

		// Generate UVs (flat mapping by default)
		if (shape.uvs.empty()) {
			Rect rect;
			rect.min.x = rect.min.y = +numeric_limits<double>::infinity();
			rect.max.x = rect.max.y = -numeric_limits<double>::infinity();
			shape.uvs.reserve(shape.points.size());
			for (auto &p : shape.points) {
				Point2D uv(p.x, p.z);
				rect |= uv;
				shape.uvs.push_back(uv);
			}
			for (auto &uv : shape.uvs) {
				uv.x = (uv.x - rect.min.x) / rect.size().x;
				uv.y = (uv.y - rect.min.y) / rect.size().y;
			}
		}
	}

    if (!conf.point_normals && shape.normals.empty()) { // TODO should already be done
        shape.normals = calculate_face_normals(shape.points, shape.faces);
    }

	auto with_cubemap = section["cubeMap"].as_bool_or_default(false);

	double scale;
	auto m = transform_from_conf(section, eye, scale);

	return convert(shape, mat, m, scale, with_cubemap, conf.point_normals);
}

static void common_conf(
	const ini::Configuration &conf,
	Color &background,
	int &size,
	Matrix4D &mat_eye,
	Matrix4D &mat_inv_eye,
	int &nr_fig,
	Frustum &frustum,
	bool &frustum_use
) {
	background = tup_to_color(conf["General"]["backgroundcolor"].as_double_tuple_or_die());
	size = conf["General"]["size"].as_int_or_die();
	auto eye = tup_to_point3d(conf["General"]["eye"].as_double_tuple_or_die());
	nr_fig = conf["General"]["nrFigures"].as_int_or_die();

	auto clipping = conf["General"]["clipping"].as_bool_or_default(false);

	// Create projection matrix
	Vector3D dir;
	if (clipping) {
		dir = tup_to_vector3d(conf["General"]["viewDirection"].as_double_tuple_or_die());
		frustum_use = true;
		frustum.near = conf["General"]["dNear"].as_double_or_die();
		frustum.far = conf["General"]["dFar"].as_double_or_die();
		frustum.fov = deg2rad(conf["General"]["hfov"].as_double_or_die());
		frustum.aspect = conf["General"]["aspectRatio"].as_double_or_die();
	} else {
		frustum_use = false;
		dir = Point3D() - eye;
	}

	mat_eye = look_direction(eye, dir, mat_inv_eye);
}

img::EasyImage wireframe(const ini::Configuration &conf, bool with_z) {
	Color bg;
	int size, nr_fig;
	Matrix4D mat_eye, mat_inv_eye;
	Frustum frustum;
	bool frustum_use;
	common_conf(conf, bg, size, mat_eye, mat_inv_eye, nr_fig, frustum, frustum_use);

	// Parse figures
	vector<LineFigure> figures;
	for (int i = 0; i < nr_fig; i++) {
		auto section = conf[string("Figure") + to_string(i)];
		auto type = section["type"].as_string_or_die();
		auto mat = transform_from_conf(section, mat_eye);
		auto color = color_from_conf(section);

		auto nogen = true;

		auto f_g = [&](auto s, void (*g)(const Configuration &, EdgeShape &)) {
			if (type == s) {
				assert(nogen);
				EdgeShape shape;
				g({ section, false }, shape);
				for (auto &p : shape.points) {
					p *= mat;
				}
				figures.push_back({
					shape.points,
					shape.edges,
					color,
				});
				nogen = false;
			}
		};
		auto f_b = [&](auto s, const auto &t) {
			if (type == s) {
				assert(nogen);
				EdgeShape shape = t;
				for (auto &p : shape.points) {
					p *= mat;
				}
				figures.push_back({
					shape.points,
					shape.edges,
					color,
				});
				nogen = false;
			}
		};
		auto f_f = [&](auto s, const auto &t) {
			if (type == s) {
				assert(nogen);
				EdgeShape shape;
				fractal({ section, false }, t, shape);
				for (auto &p : shape.points) {
					p *= mat;
				}
				figures.push_back({
					shape.points,
					shape.edges,
					color,
				});
				nogen = false;
			}
		};
		auto f_t = [&](auto s, const auto &f) {
			if (type == s) {
				assert(nogen);
				EdgeShape thick;
				thicken({ section, false }, f, thick);
				for (auto &p : thick.points) {
					p *= mat;
				}
				figures.push_back({ thick.points, thick.edges, color });
				nogen = false;
			}
		};

		f_b("BuckyBall", buckyball);
		f_b("Cube", cube);
		f_b("Dodecahedron", dodecahedron);
		f_b("Icosahedron", icosahedron);
		f_b("Octahedron", octahedron);
		f_b("Tetrahedron", tetrahedron);

		f_g("LineDrawing", [](auto a, auto b) { return wireframe::line_drawing(a.section, b); });
		f_g("Cylinder", cylinder);
		f_g("Cone", cone);
		f_g("MengerSponge", mengersponge);
		f_g("Sphere", sphere);
		f_g("Torus", torus);
		f_g("3DLSystem", [](auto a, auto b) { return wireframe::l_system(a.section, b); });

		f_f("FractalCube", cube);
		f_f("FractalOctahedron", octahedron);
		f_f("FractalTetrahedron", tetrahedron);
		f_f("FractalIcosahedron", icosahedron);
		f_f("FractalDodecahedron", dodecahedron);
		f_f("FractalBuckyBall", buckyball);

		if (type == "ThickLineDrawing") {
			EdgeShape templ, thick;
			wireframe::line_drawing(section, templ);
			thicken({ section, false }, ShapeTemplateAny(templ), thick);
			for (auto &p : thick.points) {
				p *= mat;
			}
			figures.push_back({ thick.points, thick.edges, color });
			nogen = false;
		}
		if (type == "Thick3DLSystem") {
			EdgeShape templ, thick;
			wireframe::l_system(section, templ);
			thicken({ section, false }, ShapeTemplateAny(templ), thick);
			for (auto &p : thick.points) {
				p *= mat;
			}
			figures.push_back({ thick.points, thick.edges, color });
			nogen = false;
		}
		f_t("ThickBuckyBall", buckyball);
		f_t("ThickCube", cube);
		f_t("ThickDodecahedron", dodecahedron);
		f_t("ThickIcosahedron", icosahedron);
		f_t("ThickOctahedron", octahedron);
		f_t("ThickTetrahedron", tetrahedron);

		if (nogen) {
			throw TypeException(type);
		}
	}

	// Draw
	return render::draw(figures, size, bg, with_z);
}

img::EasyImage triangles(const ini::Configuration &conf, bool with_lighting) {
	Color bg;
	int size, nr_fig;
	Frustum frustum;
	bool frustum_use;
	Lights lights;

	common_conf(conf, bg, size, lights.eye, lights.inv_eye, nr_fig, frustum, frustum_use);

	// Parse lights
	if (with_lighting) {
		lights.shadows = conf["General"]["shadowEnabled"].as_bool_or_default(false);
		lights.shadow_mask = lights.shadows ? conf["General"]["shadowMask"].as_int_or_die(): 0;

		int nr_light = conf["General"]["nrLights"];
		for (int i = 0; i < nr_light; i++) {
			auto section = conf[string("Light") + to_string(i)];
			lights.ambient += color_from_conf(section["ambientLight"]);
			vector<double> diffuse, specular;
			if (section["diffuseLight"].as_double_tuple_if_exists(diffuse)
				| section["specularLight"].as_double_tuple_if_exists(specular)) {
				if (section["infinity"].as_bool_or_default(false)) {
					lights.directional.push_back({
						tup_to_vector3d(section["direction"].as_double_tuple_or_die()).normalize() * lights.eye,
						try_color_from_conf(diffuse),
						try_color_from_conf(specular),
					});
				} else {
					auto d = tup_to_point3d(section["location"].as_double_tuple_or_die());
					auto a = deg2rad(section["spotAngle"].as_double_or_default(90)); // 91 to ensure >= 1.0 works
					d *= lights.eye;
					lights.point.push_back({
						d,
						try_color_from_conf(diffuse),
						try_color_from_conf(specular),
						cos(a),
						{ Matrix4D(), ZBuffer(0, 0), NAN, Vector2D() },
					});
				}
			}
		}
	} else {
		lights.ambient = { 1, 1, 1 };
		lights.shadows = false;
	}
#if GRAPHICS_DEBUG_LIGHT > 0
	{
		lights.ambient = { 1, 1, 1 };
		lights.shadows = false;
		for (auto &p : lights.point) {
			auto pt = p.point * lights.inv_eye;
			lights.eye = look_direction(pt, -(pt - Point3D()));
			break;
		}
		if (lights.shadows) {
			size = lights.shadow_mask;
		}
		size = lights.shadow_mask;
		lights.directional.clear();
		lights.point.clear();
		lights.shadows = false;
	}
#endif

	// Check for cubemap
	{
		string path;
		if (conf["General"]["cubeMap"].as_string_if_exists(path)) {
			ifstream f(path);
			img::EasyImage img;
			f >> img;
			lights.cubemap.emplace(Texture(std::move(img)));
			lights.cubemap_size = conf["General"]["cubeMapSize"].as_double_or_die();
		}
	}

	// Parse figures
	vector<TriangleFigure> figures;
	figures.reserve(nr_fig);
	for (int i = 0; i < nr_fig; i++) {
		cout << "Loading Figure" << i << endl;
		auto section = conf[string("Figure") + to_string(i)];
		auto type = section["type"].as_string_or_die();
		FaceShape shape;
		auto smooth = section["smooth"].as_bool_or_default(false);
		bool nogen = true;
		Material mat;

		auto f_b = [&](auto s, const auto &t) {
			if (type == s) {
				assert(nogen);
				shape = FaceShape(t, smooth);
				nogen = false;
			}
		};
		auto f_g = [&](auto s, void (*g)(const Configuration &, FaceShape &)) {
			if (type == s) {
				assert(nogen);
				g({ section, smooth }, shape);
				nogen = false;
			}
		};
		auto f_f = [&](auto s, const auto &t) {
			if (type == s) {
				assert(nogen);
				fractal({ section, smooth }, t, shape);
				nogen = false;
			}
		};
		auto f_t = [&](auto s, const auto &f) {
			if (type == s) {
				assert(nogen);
				thicken({ section, smooth }, f, shape);
				nogen = false;
			}
		};

		f_b("BuckyBall", buckyball);
		f_b("Cube", cube);
		f_b("Tetrahedron", tetrahedron);
		f_b("Octahedron", octahedron);
		f_b("Icosahedron", icosahedron);
		f_b("Dodecahedron", dodecahedron);
		f_g("Cylinder", cylinder);
		f_g("Cone", cone);
		f_g("MengerSponge", mengersponge);
		f_g("Sphere", sphere);
		f_g("Torus", torus);
		f_f("FractalBuckyBall", buckyball);
		f_f("FractalCube", cube);
		f_f("FractalTetrahedron", tetrahedron);
		f_f("FractalOctahedron", octahedron);
		f_f("FractalIcosahedron", icosahedron);
		f_f("FractalDodecahedron", dodecahedron);

		if (type == "ThickLineDrawing") {
			EdgeShape templ;
			wireframe::line_drawing(section, templ);
			thicken({ section, smooth }, ShapeTemplateAny(templ), shape);
			nogen = false;
		}
		if (type == "Thick3DLSystem") {
			EdgeShape templ;
			wireframe::l_system(section, templ);
			thicken({ section, smooth }, ShapeTemplateAny(templ), shape);
			nogen = false;
		}
		f_t("ThickBuckyBall", buckyball);
		f_t("ThickCube", cube);
		f_t("ThickDodecahedron", dodecahedron);
		f_t("ThickIcosahedron", icosahedron);
		f_t("ThickOctahedron", octahedron);
		f_t("ThickTetrahedron", tetrahedron);

		if (type == "Object") {
			assert(nogen);
			wavefront({ section, smooth }, shape, mat, smooth);
			nogen = false;
		}

		if (nogen) {
			throw TypeException(type);
		}

		figures.push_back(convert(shape, mat, { section, smooth }, with_lighting, lights.eye));
	}

	if (lights.shadows) {
		// We need the full objects for shadowing
		lights.zfigures = ZBufferTriangleFigure::convert(figures);
	}

	// Clipping
	if (frustum_use) {
		for (auto &f : figures) {
			frustum.clip(f);
		}
	}

	// Draw
	cout << "Drawing" << endl;
	return draw(std::move(figures), lights, size, bg);
}

}
}
