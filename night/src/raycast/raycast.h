#pragma once

#include "math/math.h"

namespace night
{

	struct NIGHT_API Ray
	{
		vec3 origin{ vec3(0, 0, 0) };
		vec3 direction{ vec3(0, 0, 0) };
	};

	struct NIGHT_API raycast
	{
		struct Result2D
		{
			u8 result;

			union
			{
				array<real, 2> thetas;
				struct
				{
					real t0;
					real t1;
				};
				real t;
			};

			//vec2 normal;

			template<s32 T = 0>
			vec2 contact(vec2 const& ray_origin, vec2 const& ray_direction) const
			{
				if constexpr (T == 0)
				{
					return ray_origin + ray_direction * t0;
				}
				else
				{
					return ray_origin + ray_direction * t1;
				}
			}
		};

		struct Result3D
		{
			u8 result;

			union
			{
				array<real, 2> thetas;
				struct
				{
					real t0;
					real t1;
				};
				real t;
			};

			union
			{
				array<vec3, 2> normals;
				struct
				{
					vec3 n0;
					vec3 n1;
				};
				vec3 normal;
			};

			template<s32 T = 0>
			vec3 contact(vec3 const& ray_origin, vec3 const& ray_direction) const
			{
				if constexpr (T == 0)
				{
					return ray_origin + ray_direction * t0;
				}
				else
				{
					return ray_origin + ray_direction * t1;
				}
			}
		};

		static Result2D plane(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal);

		static Result3D plane(const vec3& ray_origin, const vec3& ray_dir, const vec3& plane_origin, const vec3& plane_normal); // TODO: better implement this.

		static Result2D circle(const vec2& ray_origin, const vec2& ray_direction, const vec2& circle_origin, real circle_radius);

		static Result3D sphere(const vec3& ray_origin, const vec3& ray_direction, const vec3& sphere_origin, real sphere_radius);

		template<typename T>
		inline static real get_ray_t(T const& point, T const& ray_origin, T const& ray_direction)
		{
			T p2 = ray_origin + ray_direction;
			T const& p1 = ray_origin;
			return dot((p2 - p1) / length(p2 - p1), (point - p1) / length(p2 - p1));
		}

		static real project_point_to_ray(const vec3& point, const vec3& ray_origin, const vec3& ray_direction);

		struct TriangleResult3D
		{
			//u8 result;
			u8 result()
			{
				return (coordinate.x >= 0.0f && coordinate.y >= 0.0f && coordinate.z >= 0.0f);
			}

			real t{INFINITY};
			vec3 coordinate{ vec3(-INFINITY) };
			vec3 normal{ vec3(0) };

			vec3 contact(vec3 const& ray_origin, vec3 const& ray_direction) const
			{
				return ray_origin + ray_direction * t;
			}
		};

		static TriangleResult3D triangle(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& p1, vec3 const& p2, vec3 const& p3);

		static Result3D cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, vec3 const& cyl_dir, real cyl_radius, real cyl_height);
		static Result3D infinite_cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, real cyl_radius, vec3 const& cyl_dir);
	};

}