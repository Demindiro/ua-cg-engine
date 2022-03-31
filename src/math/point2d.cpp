#include "math/point2d.h"
#include <ostream>

using namespace std;

ostream &operator <<(ostream &o, const Point2D &m) {
	return o << '(' << m.x << ", " << m.y << ')';
}
