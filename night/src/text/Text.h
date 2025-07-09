#pragma once

//#include "stl/stl.h"
//#include "texture/ITexture.h"
#include "ref/ref.h"

namespace night
{
	struct ITexture;

	enum class ETextOrigin : s32
	{
		TopLeft = 0,
		TopRight,
		BottomRight,
		BottomLeft,
		TopCenter,
		BottomCenter,
		LeftCenter,
		RightCenter,
		Centered
	};

	struct Text
	{
		string text;
		ref<ITexture> font{ nullptr };
		mat4 transform{ mat4(1) };
		ETextOrigin origin{ ETextOrigin::TopLeft };
		Color color{ WHITE };
	};

}