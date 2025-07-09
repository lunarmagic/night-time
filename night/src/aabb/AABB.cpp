
#include "nightpch.h"
#include "AABB.h"
#include "math/math.h"
#include "stl/stl.h"
#include "geometry/Quad.h"
//#include "debug_renderer/DebugRenderer.h"

namespace night
{

	u8 AABB::contains(vec2 const& point)
	{
		return (point.x > left && point.x < right && point.y < top && point.y > bottom);
	}

	vec2 AABB::local_coordinate(vec2 const& global_coordinate)
	{
		//vec2 result;
		//result.x = ((global_coordinate.x - left) / width()) * 2.0f - 1.0f;
		//result.y = ((global_coordinate.y - bottom) / height()) * 2.0f - 1.0f;
		//return result;

		real x = ((global_coordinate.x - left) / width()) * 2.0f - 1.0f;
		real y = ((global_coordinate.y - bottom) / height()) * 2.0f - 1.0f;

		return
		{
			 x,
			 y
		};
	}

	void AABB::fit_around_point(const vec2& point)
	{
		left = MIN(point.x, left);
		right = MAX(point.x, right);
		top = MAX(point.y, top);
		bottom = MIN(point.y, bottom);

		left = MIN(point.x, left);
		right = MAX(point.x, right);
		top = MAX(point.y, top);
		bottom = MIN(point.y, bottom);
	}

	void AABB::fit_around_quad(Quad const& quad)
	{
		for (const auto& i : quad.vertices)
		{
			left = MIN(left, i.point.x);
			right = MAX(right, i.point.x);
			top = MAX(top, i.point.y);
			bottom = MIN(bottom, i.point.y);
		}
	}

}
