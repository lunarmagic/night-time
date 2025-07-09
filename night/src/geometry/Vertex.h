#pragma once

#include "core.h"
#include "color/Color.h"
#include "math/math.h"

namespace night
{

	struct Vertex
	{
		vec4 point;
		Color color;
		vec2 texture_coord;
	};

}