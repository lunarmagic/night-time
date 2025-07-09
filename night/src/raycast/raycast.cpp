
#include "nightpch.h"
#include "raycast.h"
#include "log/log.h"

#include "glm/gtx/norm.hpp"

//#include <optional>
//#include <cmath>

namespace night
{
	raycast::Result2D raycast::plane(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal)
	{
		raycast::Result2D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = INFINITY;

		real d2 = dot(ray_dir, plane_normal);
		if (d2 == 0)
		{
			return result;
		}

		real d1 = dot(plane_origin - ray_origin, plane_normal);
		result.result = true;
		result.t0 = d1 / d2;
		result.t1 = INFINITY;
		return result;
		//return { .result = true, .t0 = d1 / d2 };
	}

	raycast::Result3D raycast::plane(const vec3& ray_origin, const vec3& ray_dir, const vec3& plane_origin, const vec3& plane_normal)
	{
		raycast::Result3D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = INFINITY;

		real d2 = dot(ray_dir, plane_normal);
		if (d2 == 0)
		{
			//return {.result = false};
			return result;
		}

		real d1 = dot(plane_origin - ray_origin, plane_normal);
		//return { .result = true, .t0 = d1 / d2 };
		result.result = true;
		result.t0 = d1 / d2;
		result.t1 = INFINITY;
		result.normal = d2 > 0.0f ? -plane_normal : plane_normal; // normal should always be against ray direction
		return result;
	}

	raycast::Result2D raycast::circle(const vec2& ray_origin, const vec2& ray_direction, const vec2& circle_origin, real circle_radius)
	{
		raycast::Result2D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = INFINITY;

		real a = dot(ray_direction, ray_direction);
		real b = 2 * dot(ray_direction, circle_origin - ray_origin);
		real c = dot(circle_origin - ray_origin, circle_origin - ray_origin);
		c -= circle_radius * circle_radius;

		real dt = b * b - 4 * a * c;

		if (dt >= 0)
		{
			real t0 = (-b - sqrt(dt)) / (a * 2);
			real t1 = (-b + sqrt(dt)) / (a * 2);

			if (t0 > t1)
			{
				SWAP(t0, t1); // TODO: remove
			}

			result.result = true;
			result.t0 = -t0; // TODO: don't know why they are inverted
			result.t1 = -t1;
			return result;
			//return { .result = true, .t0 = t0, .t1 = t1 }; // TODO: may want to swap t0 and t1 if t0 > t1
		}

		//return { .result = false };
		return result;
	}

	raycast::Result3D raycast::sphere(const vec3& ray_origin, const vec3& ray_direction, const vec3& sphere_origin, real sphere_radius)
	{
		raycast::Result3D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = INFINITY;

		real a = dot(ray_direction, ray_direction);
		real b = 2 * dot(ray_direction, sphere_origin - ray_origin);
		real c = dot(sphere_origin - ray_origin, sphere_origin - ray_origin);
		c -= sphere_radius * sphere_radius;

		real dt = b * b - 4 * a * c;

		if (dt >= 0)
		{
			real t0 = (-b - sqrt(dt)) / (a * 2);
			real t1 = (-b + sqrt(dt)) / (a * 2);

			if (t0 > t1)
			{
				SWAP(t0, t1); // TODO: remove
			}

			//return { .result = true, .t0 = t0, .t1 = t1 };
			result.result = true;
			result.t0 = -t1; // TODO: don't know why they are inverted
			result.t1 = -t0;
			vec3 c0 = ray_origin + ray_direction * result.t0;
			vec3 c1 = ray_origin + ray_direction * result.t1;
			result.n0 = normalize(c0 - sphere_origin); // TODO: can probably div by radius
			result.n1 = normalize(c1 - sphere_origin);
			return result;
		}

		//return { .result = false };
		result.result = false;
		return result;
	}

	real raycast::project_point_to_ray(const vec3& point, const vec3& ray_origin, const vec3& ray_direction)
	{
		vec3 proj = project_point_to_line(point, ray_origin, ray_origin + ray_direction);
		return get_ray_t(point, ray_origin, ray_direction);
	}

#define RAYCAST_TRIANGLE_EPSILON 0.001f
	raycast::TriangleResult3D raycast::triangle(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& p1, vec3 const& p2, vec3 const& p3)
	{
		vec3 n = cross(p2 - p1, p3 - p2);

		auto rcp = raycast::plane(ray_origin, ray_dir, p1, n);
		if (!rcp.result)
		{
			return {.t = -INFINITY, .coordinate = vec3(-INFINITY), .normal = vec3(0)};
		}

		vec3 cx = cross(p3 - p2, n);
		if (dot(cx, p3 - p1) < 0.0f)
		{
			cx = -cx;
		}

		vec3 cy = cross(p1 - p3, n);
		if (dot(cy, p1 - p2) < 0.0f)
		{
			cy = -cy;
		}

		vec3 cz = cross(p2 - p1, n);
		if (dot(cz, p2 - p3) < 0.0f)
		{
			cz = -cz;
		}

		raycast::TriangleResult3D result;

		vec3 contact = rcp.contact(ray_origin, ray_dir);
		result.t = rcp.t0;
		result.coordinate.x = -distance_to_plane(contact, p2, cx);
		result.coordinate.y = -distance_to_plane(contact, p3, cy);
		result.coordinate.z = -distance_to_plane(contact, p1, cz);
		result.normal = normalize(dot(n, ray_dir) > 0 ? -n : n); // normal should always be against ray direction
		
		return result;
//#endif

		//if (
		//	distance_to_plane(result.contact(ray_origin, ray_dir), a, abn) > 0.0f ||
		//	distance_to_plane(result.contact(ray_origin, ray_dir), b, bcn) > 0.0f ||
		//	distance_to_plane(result.contact(ray_origin, ray_dir), c, can) > 0.0f)
		//{
		//	//result.result = false;
		//	//return result;
		//}

		//return result;
	}

	raycast::Result3D raycast::cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, vec3 const& cyl_dir, real cyl_radius, real cyl_height )
	{
		raycast::Result3D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = INFINITY;

		vec3 cap_a = cyl_origin + cyl_dir * cyl_height;
		vec3 cap_b = cyl_origin - cyl_dir * cyl_height;

		vec3 AB = (cap_b - cap_a);
		vec3 AO = (ray_origin - cap_a);
		vec3 AOxAB = (cross(AO, AB));
		vec3 VxAB = (cross(ray_dir, AB));
		real ab2 = dot(AB, AB);
		real A = dot(VxAB, VxAB);
		real B = 2 * dot(VxAB, AOxAB);
		real C = (dot(AOxAB, AOxAB) - (cyl_radius * cyl_radius * ab2));

		real discr = B * B - 4 * A * C;

		if (discr < 0) return result;

		real t1 = (-B + sqrt(discr)) / (2 * A);
		real t2 = (-B - sqrt(discr)) / (2 * A);

		if (t1 > t2)
		{
			SWAP(t1, t2);
		}

		vec3 contact1 = ray_origin + ray_dir * t1;
		real c1a = distance_to_plane(contact1, cap_a, cyl_dir);
		real c1b = distance_to_plane(contact1, cap_b, -cyl_dir);
		u8 c1ax = (c1a < 0);
		u8 c1bx = (c1b < 0);

		real min_t = INFINITY;
		if (c1ax && c1bx)
		{
			// normal is on cylinder body
			vec3 pptl = project_point_to_line(contact1, cyl_origin, cyl_origin + cyl_dir);
			result.n0 = normalize(contact1 - pptl);
			min_t = t1;
		}
		else
		{
			if (!c1ax)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_a, cyl_dir);
				ASSERT(rc.result); // TODO: handle invalid rc
				vec3 contact = ray_origin + ray_dir * rc.t0;
				real d = distance(contact, cap_a);
				if (d < cyl_radius)
				{
					result.n0 = rc.normal; // normal of t0 is cap a normal
					min_t = MIN(min_t, rc.t0);
				}
			}
			if (!c1bx)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_b, -cyl_dir);
				ASSERT(rc.result); // TODO: handle invalid rc
				vec3 contact = ray_origin + ray_dir * rc.t0;
				real d = distance(contact, cap_b);
				if (d < cyl_radius)
				{
					result.n0 = rc.normal; // normal of t0 is cap b normal
					min_t = MIN(min_t, rc.t0);
				}
			}
		}

		vec3 contact2 = ray_origin + ray_dir * t2;
		real c2a = distance_to_plane(contact2, cap_a, cyl_dir);
		real c2b = distance_to_plane(contact2, cap_b, -cyl_dir);
		u8 c2ax = (c2a < 0);
		u8 c2bx = (c2b < 0);
		real max_t = -INFINITY;

		if (c2ax && c2bx)
		{
			// normal is on cylinder body
			vec3 pptl = project_point_to_line(contact1, cyl_origin, cyl_origin + cyl_dir);
			result.n1 = normalize(contact1 - pptl);
			max_t = t2;
		}
		else
		{
			if (!c2ax)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_a, cyl_dir);
				ASSERT(rc.result); // TODO: handle invalid rc
				vec3 contact = ray_origin + ray_dir * rc.t0;
				real d = distance(contact, cap_a);
				if (d < cyl_radius)
				{
					result.n1 = rc.normal; // normal of t1 is cap a normal
					max_t = MAX(max_t, rc.t0);
				}
			}
			if (!c2bx)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_b, -cyl_dir);
				ASSERT(rc.result); // TODO: handle invalid rc
				vec3 contact = ray_origin + ray_dir * rc.t0;
				real d = distance(contact, cap_b);
				if (d < cyl_radius)
				{
					result.n1 = rc.normal; // normal of t1 is cap b normal
					max_t = MAX(max_t, rc.t0);
				}
			}
		}

		result.result = min_t != INFINITY;
		result.t0 = min_t;
		result.t1 = max_t;

		//vec3 c0 = ray_origin + ray_dir * result.t0;
		//vec3 c1 = ray_origin + ray_dir * result.t1;
		//vec3 proj_1 = project_point_to_line(c0, cyl_origin, cyl_origin + cyl_dir);

		return result;
	}

	raycast::Result3D raycast::infinite_cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, real cyl_radius, vec3 const& cyl_dir)
	{
		ASSERT(false);// TODO: implement
		return Result3D();
	}

}