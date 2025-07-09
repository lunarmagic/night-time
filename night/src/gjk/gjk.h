#pragma once

//#include "log/log.h"
#include "math/math.h"

namespace night
{

	struct ShapeCastResult3D
	{
		//u8 result{ false };
		//real t{ INFINITY };
		//vec3 normal{ vec3(0) };

		u8 result;

		union
		{
			array<real, 2> thetas;
			struct
			{
				real t0;
				real t1;
			};
			//real t;
		};

		union
		{
			array<vec3, 2> normals;
			struct
			{
				vec3 n0;
				vec3 n1;
			};
			//vec3 normal;
		};
	};

	struct ShapeCastParams3D
	{
		vec3 motion;
		function<vec3(vec3 const&)> support_casted;
		function<vec3(vec3)> support_against;
		real epsilon = 0.0001f;
		s32 max_iterations = 32;
	};

	struct NIGHT_API gjk
	{
		static u8 intersects(function<vec3(vec3 const&)> const& support_a, function<vec3(vec3 const&)> const& support_b, real epsilon = 0.0001f, s32 max_iterations = 32);

		static ShapeCastResult3D shape_cast(ShapeCastParams3D const& params, u8 skip_t1 = false);

	private:

		static ShapeCastResult3D shape_cast_impl(ShapeCastParams3D const& params, u8 skip_t1);
	};

}