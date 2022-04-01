#pragma once

#include "easy_image.h"
#include "ini_configuration.h"

namespace engine {
namespace intro {
	img::EasyImage color_rectangle(const ini::Configuration &);
	img::EasyImage blocks(const ini::Configuration &);
	img::EasyImage lines(const ini::Configuration &);
}
}
