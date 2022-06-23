#include "render/rect.h"
#include <ostream>

namespace engine {
namespace render {

std::ostream &operator <<(std::ostream &out, const Rect &rect) {
	return out << '(' << rect.min << ", " << rect.max << ')';
}

}
}
