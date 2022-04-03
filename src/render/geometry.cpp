#include "render/geometry.h"
#include <cassert>
#include "render/triangle.h"

namespace engine {
namespace render {

/**
 * \brief Apply frustum clipping.
 */
template<typename B, typename P>
static void frustum_apply(TriangleFigure &f, B bitfield, P project, bool disable_cull) {
	assert((f.faces.size() == f.normals.size() && "faces & normals out of sync"));
	size_t faces_count = f.faces.size();
	size_t added = 0;
	auto proj = [&f, &project](auto from_i, auto to_i) {
		assert(from_i < f.points.size());
		assert(to_i < f.points.size());
		auto from = f.points[from_i];
		auto to = f.points[to_i];
		auto p = to.interpolate(from, project(from, to));
		f.points.push_back(p);
		return (unsigned int)(f.points.size() - 1);
	};
	auto swap_remove = [&f, &faces_count, disable_cull](size_t i) {
		// Swap, then remove. This is always O(1)
		if (i < faces_count) {
			f.faces[i] = f.faces[faces_count - 1];
			if (f.normals.size() > 0)
				f.normals[i] = f.normals[faces_count - 1];
		}
		faces_count--;
		if (disable_cull) {
			f.set_flag(TriangleFigure::can_cull, false);
		}
		f.set_flag(TriangleFigure::clipped, true);
	};
	auto split = [&proj, &f, &added, disable_cull](auto i, auto &out, auto inl, auto inr) {
		auto p = proj(out, inl);
		auto q = proj(out, inr);
		out = p;
		f.faces.push_back({ p, q, inr });
		if (f.normals.size() > 0)
			f.normals.push_back(f.normals[i]);
		added++;
		if (disable_cull) {
			f.set_flag(TriangleFigure::can_cull, false);
		}
		f.set_flag(TriangleFigure::clipped, true);
	};
	for (size_t i = 0; i < faces_count; i++) {
		auto &t = f.faces[i];
		switch(bitfield(t)) {
		// Nothing to do
		case 0b000:
			break;
		// Split triangle
		case 0b100:
			split(i, t.a, t.b, t.c);
			break;
		case 0b010:
			split(i, t.b, t.c, t.a);
			break;
		case 0b001:
			split(i, t.c, t.a, t.b);
			break;
		// Shrink triangle
		case 0b011:
			t.b = proj(t.b, t.a);
			t.c = proj(t.c, t.a);
			f.set_flag(TriangleFigure::clipped, true);
			break;
		case 0b101:
			t.c = proj(t.c, t.b);
			t.a = proj(t.a, t.b);
			f.set_flag(TriangleFigure::clipped, true);
			break;
		case 0b110:
			t.a = proj(t.a, t.c);
			t.b = proj(t.b, t.c);
			f.set_flag(TriangleFigure::clipped, true);
			break;
		// Remove triangle
		case 0b111:
			swap_remove(i--);
			break;
		default:
			UNREACHABLE;
		}
	}

	// Copy added faces over deleted faces.
	{
		size_t i = faces_count, j = f.faces.size() - added;
		size_t new_size = faces_count + added;
		assert(i <= j);
		while (added --> 0) {
			f.faces[i] = f.faces[j];
			if (f.normals.size() > 0)
				f.normals[i] = f.normals[j];
			i++, j++;
		}
		f.faces.resize(new_size);
		if (f.normals.size() > 0)
			f.normals.resize(new_size);
	}
}

void Frustum::clip(TriangleFigure &f) const {
	// Near & far plane
	enum Plane {
		NEAR,
		FAR,
		RIGHT,
		LEFT,
		TOP,
		DOWN,
	};
	auto outside = [this](Point3D p, int plane, double v) {
		switch (plane) {
			case NEAR : return -p.z < near;
			case FAR  : return -p.z > far;
			case RIGHT: return p.x * near > v * -p.z;
			case LEFT : return p.x * near < v * -p.z;
			case TOP  : return p.y * near > v * -p.z;
			case DOWN : return p.y * near < v * -p.z;
			default:
				assert(!"Invalid direction");
				return false;
		}
	};
	auto outside_mask = [&f, &outside](Face &t, int plane, double v) {
		assert(t.a < f.points.size());
		assert(t.b < f.points.size());
		assert(t.c < f.points.size());
		return (int)outside(f.points[t.a], plane, v) << 2
			| (int)outside(f.points[t.b], plane, v) << 1
			| (int)outside(f.points[t.c], plane, v);
	};

	// Near & far
	auto dnear = near, dfar = far;
	{
		frustum_apply(f,
			[&outside_mask](auto &t) { return outside_mask(t, NEAR, NAN); },
			[dnear, dfar](Point3D from, Point3D to) {
				return (-dnear - to.z) / (from.z - to.z);
			},
			true
		);
		frustum_apply(f,
			[&outside_mask](auto &t) { return outside_mask(t, FAR, NAN); },
			[dnear, dfar](Point3D from, Point3D to) {
				return (-dfar - to.z) / (from.z - to.z);
			},
			false
		);
	}

	// Left & right plane
	auto right = dnear * tan(fov / 2);
	{
		frustum_apply(f,
			[&outside_mask, right](auto &t) { return outside_mask(t, RIGHT, right); },
			[dnear, right](Point3D from, Point3D to) {
				return (to.x * dnear + to.z * right) /
					((to.x - from.x) * dnear + (to.z - from.z) * right);
			},
			false
		);
		frustum_apply(f,
			[&outside_mask, right](auto &t) { return outside_mask(t, LEFT, -right); },
			[dnear, right](Point3D from, Point3D to) {
				return (to.x * dnear + to.z * -right) /
					((to.x - from.x) * dnear + (to.z - from.z) * -right);
			},
			false
		);
	}

	// Top & down plane
	auto top = right / aspect;
	{
		frustum_apply(f,
			[&outside_mask, top](auto &t) { return outside_mask(t, TOP, top); },
			[dnear, top](Point3D from, Point3D to) {
				return (to.y * dnear + to.z * top) /
					((to.y - from.y) * dnear + (to.z - from.z) * top);
			},
			false
		);
		frustum_apply(f,
			[&outside_mask, top](auto &t) { return outside_mask(t, DOWN, -top); },
			[dnear, top](Point3D from, Point3D to) {
				return (to.y * dnear + to.z * -top) /
					((to.y - from.y) * dnear + (to.z - from.z) * -top);
			},
			false
		);
	}
}

}
}
