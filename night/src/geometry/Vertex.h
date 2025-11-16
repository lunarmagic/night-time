#pragma once

#include "core.h"
#include "color/Color.h"
#include "math/Math.h"

namespace night
{

	template<typename T = real>
	struct Vertex
	{
		vec<4, T> point;
		Color color;
		vec<2, r32> texture_coord;
	};

}