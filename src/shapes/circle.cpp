#include "shapes/circle.h"
#include <cmath>
#include "lines.h"

using namespace std;

void shapes::circle(vector<Vector3D> &points, int n, double z) {
	double d = 2 * M_PI / n;
	for (int i = 0; i < n; i++) {
		auto x = sin(i * d);
		auto y = cos(i * d);
		points.push_back(Vector3D::point(x, y, z));
	}
}

void shapes::circle(vector<Face> &faces, int n, int offt) {
	for (int i = 0; i < n - 2; i++) {
		faces.push_back({ offt, offt + i + 1, offt + i + 2 });
	}
}
