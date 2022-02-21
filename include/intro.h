#pragma once

#include "easy_image.h"
#include "ini_configuration.h"

namespace intro {
	void color_rectangle(img::EasyImage &, const ini::Configuration &);
	void blocks(img::EasyImage &, const ini::Configuration &);
	void lines(img::EasyImage &, const ini::Configuration &);
}
