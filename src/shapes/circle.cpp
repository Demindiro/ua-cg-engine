#include "shapes/circle.h"
#include <cmath>
#include "engine.h"
#include "lines.h"

using namespace std;

void shapes::circle(vector<Point3D> &points, unsigned int n, double z) {
	Rotation d(2 * M_PI / n), r;
	for (unsigned int i = 0; i < n; i++) {
		points.push_back({ r.u, r.v, z });
		r *= d;
	}
}

void shapes::circle(vector<Face> &faces, unsigned int n, unsigned int offt) {
	for (unsigned int i = 0; i < n - 2; i++) {
		faces.push_back({ offt + i + 1, offt + i + 2, offt });
	}
}

void shapes::circle_reversed(vector<Face> &faces, unsigned int n, unsigned int offt) {
	for (unsigned int i = 0; i < n - 2; i++) {
		faces.push_back({ offt + i + 2, offt + i + 1, offt });
	}
}
