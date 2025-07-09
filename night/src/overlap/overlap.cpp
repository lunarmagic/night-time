
#include "nightpch.h"
#include "overlap.h"
//#include "math/math.h"
//#include "debug_renderer/DebugRenderer.h"
#include "raycast/raycast.h"


namespace night
{
	u8 overlap::infinite_cone(vec3 const point, vec3 const& cone_origin, vec3 const& cone_dir, real cone_radius, real cone_height)
	{
		vec3 cone_tip = cone_origin - cone_dir * cone_height;
		real t = raycast::project_point_to_ray(point, cone_tip, cone_dir * cone_height * 2.0f);
		vec3 p = cone_tip + (cone_dir * cone_height * 2.0f) * t;
		real dist = distance(point, p);
		real radius = cone_radius * t;

		return (dist < radius);
	}
}


