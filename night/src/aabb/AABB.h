#pragma once

//#include "utility.h"
#include "core.h"
#include "math/math.h"
//#include "debug_renderer/DebugRenderer.h"

namespace night
{

	struct Quad;

	struct NIGHT_API AABB
	{
		//AABB() = default;
		//AABB(Quad const& quad);

		real left{ INFINITY };
		real right{ -INFINITY };
		real top{ -INFINITY }; // TODO up is +.
		real bottom{ INFINITY };

		void fit_around_point(const vec2& point);
		void fit_around_quad(Quad const& quad);
		u8 contains(vec2 const& point);
		real width() const { return right - left; }
		real height() const { return top - bottom; }

		vec2 local_coordinate(vec2 const& global_coordinate);
	};

}
