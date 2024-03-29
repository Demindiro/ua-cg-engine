#pragma once

#include <memory>
#include "easy_image.h"
#include "math/point2d.h"

namespace engine {
namespace render {

struct Texture {
	std::shared_ptr<img::EasyImage> image;

	Texture(img::EasyImage &&img) : image(std::make_shared<img::EasyImage>(std::move(img))) {}

	img::Color get_clamped(Point2D uv) const {
		unsigned int u = round_up((image->get_width() - 1) * std::clamp(uv.x, 0.0, 1.0));
		unsigned int v = round_up((image->get_height() - 1) * std::clamp(uv.y, 0.0, 1.0));
		return (*image)(u, v);
	}
};

}
}
