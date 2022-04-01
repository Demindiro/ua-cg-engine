#include "shapes.h"
#include <cmath>
#include "engine.h"
#include "lines.h"
#include "math/point3d.h"
#include "shapes/circle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

void circle(vector<Point3D> &points, unsigned int n, double z) {
	Rotation d(-2 * M_PI / n), r;
	for (unsigned int i = 0; i < n; i++) {
		points.push_back({ r.u, r.v, z });
		r *= d;
	}
}

void circle(vector<Face> &faces, unsigned int n, unsigned int offt) {
	for (unsigned int i = 0; i < n - 2; i++) {
		faces.push_back({ offt + i + 1, offt + i + 2, offt });
	}
}

void circle_reversed(vector<Face> &faces, unsigned int n, unsigned int offt) {
	for (unsigned int i = 0; i < n - 2; i++) {
		faces.push_back({ offt + i + 2, offt + i + 1, offt });
	}
}

}
}
