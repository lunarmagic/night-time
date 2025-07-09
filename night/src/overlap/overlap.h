#pragma once

#include "math/math.h"

namespace night
{

	struct NIGHT_API overlap
	{
		//static u8 cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, vec3 const& cyl_dir, real cyl_height, real cyl_radius);
		//static u8 cone(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cone_origin, vec3 const& cone_dir, real cone_height, real cone_radius);

		//static u8 infinite_cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, vec3 const& cyl_dir, real cyl_radius);
		static u8 infinite_cone(vec3 const point, vec3 const& cone_origin, vec3 const& cone_dir, real cone_radius, real cone_height);
	};

}