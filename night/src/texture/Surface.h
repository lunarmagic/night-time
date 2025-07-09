#pragma once

#include "core.h"
#include "color/Color.h"
//#include "stl/stl.h"

namespace night
{

	struct SurfaceParams
	{
		s32 width{};
		s32 height{};
		Color fill_color{ WHITE };
		string path{ "" };
		u8 flip_vertically{ false };
	};

	struct NIGHT_API Surface
	{
		Surface(const SurfaceParams& params);
		~Surface();

		s32 width() const { return _width; }
		s32 height() const { return _height; }
		s32 channels() const { return _channels; }

		//virtual Color8* pixels() const = 0;

		Color8* const& pixels() const { return _pixels; }

	protected:

		void width(s32 const& w) { _width = w; }
		void height(s32 const& h) { _height = h; }
		//void channels(s32 const& c) { _channels = c; }

	private:

		s32 _width{ 0 };
		s32 _height{ 0 };
		s32 _channels{ 0 };
		Color8* _pixels{ nullptr };
	};

}