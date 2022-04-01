#include "render/color.h"
#include <ostream>

namespace engine {
namespace render {

using namespace std;

ostream &operator <<(ostream &o, const Color &c) {
	return o << '(' << c.r << ", " << c.g << ", " << c.b << ')';
}

}
}
