
#include "nightpch.h"
#include "Surface.h"
#include "log/log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

namespace night
{
	Surface::Surface(const SurfaceParams& params)
	{
		stbi_set_flip_vertically_on_load(!params.flip_vertically);

		if (!params.path.empty())
		{
			// TODO: load from path
			//s32 width, height, nrChannels;
			_pixels = (Color8*)stbi_load(params.path.c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);

			if (stbi_failure_reason())
			{
				ERROR(stbi_failure_reason());
				return;
			}

			//_width = width;
			//_height = height;
		}
		else
		{
			ASSERT(params.width > 0 && params.height > 0); // empty surface..
			_pixels = new Color8[params.width * params.height]; // TODO: handle this with stb_image

			auto color_8 = Color8(params.fill_color);
			for (s32 i = 0; i < params.width * params.height; i++)
			{
				_pixels[i] = color_8;
			}

			_width = params.width;
			_height = params.height;
		}
	}

	Surface::~Surface()
	{
		ASSERT(_pixels != nullptr);
		stbi_image_free(_pixels);
	}
}