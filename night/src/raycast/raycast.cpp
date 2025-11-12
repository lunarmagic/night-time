
#include "nightpch.h"
#include "raycast.h"
#include "log/log.h"

#include "glm/gtx/norm.hpp"

//#include <optional>
//#include <cmath>

namespace night
{
#if 0
	raycast::Result2D raycast::plane(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal, real epsilon)
	{
		raycast::Result2D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		real d2 = math::dot(ray_dir, plane_normal);
		if(abs(d2) <= epsilon)
		{
			return result;
		}

		real d1 = math::dot(plane_origin - ray_origin, plane_normal);
		result.result = true;
		result.t0 = d1 / d2;
		//result.t1 = INFINITY;
		result.normal = plane_normal;
		return result;
	}

	raycast::Result3D raycast::plane(const vec3& ray_origin, const vec3& ray_dir, const vec3& plane_origin, const vec3& plane_normal, real epsilon)
	{
		raycast::Result3D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		real d2 = math::dot(ray_dir, plane_normal);
		if (abs(d2) <= epsilon)
		{
			return result;
		}

		real d1 = math::dot(plane_origin - ray_origin, plane_normal);
		result.result = true;
		result.t0 = d1 / d2;
		//result.t1 = INFINITY;
		result.normal = d2 > 0.0f ? -plane_normal : plane_normal; // normal should always be against ray direction
		return result;
	}

	// TODO: optimize this function.
	raycast::Result2D raycast::line(vec2 const& ray_origin, vec2 const& ray_dir, vec2 const& p1, vec2 const& p2, real epsilon)
	{
		vec2 line_direction = p2 - p1;
		vec2 ray_normal = { -ray_dir.y, ray_dir.x };
		vec2 line_normal = { -line_direction.y, line_direction.x };

		Result2D rc1 = raycast::plane(ray_origin, ray_dir, p1, line_normal, epsilon);
		Result2D rc2 = raycast::plane(p1, line_direction, ray_origin, ray_normal, epsilon);

		rc1.normal = line_normal;

		if (rc2.t < 0.0f || rc2.t > 1.0f)
		{
			rc1.result = false;
		}

		return rc1;
	}

	raycast::Result2D raycast::circle(const vec2& ray_origin, const vec2& ray_direction, const vec2& circle_origin, real circle_radius)
	{
		raycast::Result2D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		real a = math::dot(ray_direction, ray_direction);
		real b = 2 * math::dot(ray_direction, circle_origin - ray_origin);
		real c = math::dot(circle_origin - ray_origin, circle_origin - ray_origin);
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
			vec2 c0 = ray_origin + ray_direction * result.t0;
			vec2 c1 = ray_origin + ray_direction * result.t1;
			result.n0 = math::normalize(c0 - circle_origin); // TODO: can probably div by radius
			result.n1 = math::normalize(c1 - circle_origin);
			return result;
		}

		return result;
	}

	raycast::Result3D raycast::sphere(const vec3& ray_origin, const vec3& ray_direction, const vec3& sphere_origin, real sphere_radius)
	{
		raycast::Result3D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		real a = math::dot(ray_direction, ray_direction);
		real b = 2 * math::dot(ray_direction, sphere_origin - ray_origin);
		real c = math::dot(sphere_origin - ray_origin, sphere_origin - ray_origin);
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

			result.result = true;
			result.t0 = -t1; // TODO: don't know why they are inverted
			result.t1 = -t0;
			vec3 c0 = ray_origin + ray_direction * result.t0;
			vec3 c1 = ray_origin + ray_direction * result.t1;
			result.n0 = math::normalize(c0 - sphere_origin); // TODO: can probably div by radius
			result.n1 = math::normalize(c1 - sphere_origin);
			return result;
		}

		result.result = false;
		return result;
	}

	vec3 raycast::project_point_to_ray(const vec3& point, const vec3& ray_origin, const vec3& ray_direction)
	{
		real t = dot(point - ray_origin, ray_direction) / dot(ray_direction, ray_direction);
		t = CLAMP(t, 0.0f, 1.0f);
		return ray_origin + ray_direction * t;
	}

#define RAYCAST_TRIANGLE_EPSILON 0.001f
	raycast::TriangleResult3D raycast::triangle(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& p1, vec3 const& p2, vec3 const& p3, real epsilon)
	{
		vec3 n = math::cross(p2 - p1, p3 - p2);

		auto rcp = raycast::plane(ray_origin, ray_dir, p1, n, epsilon);
		if (!rcp.result)
		{
			return {.t = -INFINITY, .coordinate = vec3(-INFINITY), .normal = vec3(0)};
		}

		vec3 cx = math::cross(p3 - p2, n);
		if (math::dot(cx, p3 - p1) < 0.0f)
		{
			cx = -cx;
		}

		vec3 cy = math::cross(p1 - p3, n);
		if (math::dot(cy, p1 - p2) < 0.0f)
		{
			cy = -cy;
		}

		vec3 cz = math::cross(p2 - p1, n);
		if (math::dot(cz, p2 - p3) < 0.0f)
		{
			cz = -cz;
		}

		raycast::TriangleResult3D result;

		vec3 contact = rcp.contact(ray_origin, ray_dir);
		result.t = rcp.t0;
		result.coordinate.x = -math::distance_to_plane(contact, p2, cx);
		result.coordinate.y = -math::distance_to_plane(contact, p3, cy);
		result.coordinate.z = -math::distance_to_plane(contact, p1, cz);
		result.normal = math::normalize(math::dot(n, ray_dir) > 0 ? -n : n); // normal should always be against ray direction
		
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

#if 0
	raycast::Result2D raycast::convex(vec2 const& ray_origin, vec2 const& ray_dir, vector<vec2> const& convex)
	{
		raycast::Result2D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = INFINITY;

		if (convex.size() < 2)
		{
			return result;
		}

		real min_t = INFINITY;
		real max_t = -INFINITY;

		for (s32 i = 0; i < convex.size(); i++)
		{
			vec2 const& p1 = convex[i];
			vec2 const& p2 = convex[(i + 1) % convex.size()];

			raycast::Result2D rc = raycast::line(ray_origin, ray_dir, p1, p2);
			if (rc.result)
			{
				if (rc.t0 < min_t)
				{
					min_t = rc.t0;
				}

				if (rc.t1 > max_t)
				{
					max_t = rc.t1;
				}
			}
		}

		result.result = (min_t != INFINITY && max_t != -INFINITY);
		result.t0 = min_t;
		result.t1 = max_t;

		return result;
	}
#endif

	raycast::Result3D raycast::cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, vec3 const& cyl_dir, real cyl_radius, real cyl_height )
	{
		raycast::Result3D result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		vec3 cap_a = cyl_origin + cyl_dir * cyl_height;
		vec3 cap_b = cyl_origin - cyl_dir * cyl_height;

		vec3 AB = (cap_b - cap_a);
		vec3 AO = (ray_origin - cap_a);
		vec3 AOxAB = (math::cross(AO, AB));
		vec3 VxAB = (math::cross(ray_dir, AB));
		real ab2 = math::dot(AB, AB);
		real A = math::dot(VxAB, VxAB);
		real B = 2 * math::dot(VxAB, AOxAB);
		real C = (math::dot(AOxAB, AOxAB) - (cyl_radius * cyl_radius * ab2));

		real discr = B * B - 4 * A * C;

		if (discr < 0)
		{
			return result;
		}

		real t1 = (-B + sqrt(discr)) / (2 * A);
		real t2 = (-B - sqrt(discr)) / (2 * A);

		if (t1 > t2)
		{
			SWAP(t1, t2);
		}

		vec3 contact1 = ray_origin + ray_dir * t1;
		real c1a = math::distance_to_plane(contact1, cap_a, cyl_dir);
		real c1b = math::distance_to_plane(contact1, cap_b, -cyl_dir);
		u8 c1ax = (c1a < 0);
		u8 c1bx = (c1b < 0);

		real min_t = INFINITY;
		if (c1ax && c1bx)
		{
			// normal is on cylinder body
			vec3 pptl = project_point_to_ray(contact1, cyl_origin, cyl_dir);
			result.n0 = math::normalize(contact1 - pptl);
			min_t = t1;
		}
		else
		{
			if (!c1ax)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_a, cyl_dir);
				if (rc.result)
				{
					vec3 contact = ray_origin + ray_dir * rc.t0;
					real d = math::length(contact - cap_a);
					if (d < cyl_radius)
					{
						result.n0 = rc.normal; // normal of t0 is cap a normal
						min_t = MIN(min_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge case");
				}
			}
			if (!c1bx)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_b, -cyl_dir);
				if (rc.result)
				{
					vec3 contact = ray_origin + ray_dir * rc.t0;
					real d = math::length(contact - cap_b);
					if (d < cyl_radius)
					{
						result.n0 = rc.normal; // normal of t0 is cap b normal
						min_t = MIN(min_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge");
				}
			}
		}

		vec3 contact2 = ray_origin + ray_dir * t2;
		real c2a = math::distance_to_plane(contact2, cap_a, cyl_dir);
		real c2b = math::distance_to_plane(contact2, cap_b, -cyl_dir);
		u8 c2ax = (c2a < 0);
		u8 c2bx = (c2b < 0);
		real max_t = -INFINITY;

		if (c2ax && c2bx)
		{
			// normal is on cylinder body
			vec3 pptl = project_point_to_ray(contact1, cyl_origin, cyl_dir);
			result.n1 = math::normalize(contact1 - pptl);
			max_t = t2;
		}
		else
		{
			if (!c2ax)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_a, cyl_dir);
				if (rc.result)
				{
					vec3 contact = ray_origin + ray_dir * rc.t0;
					real d = math::length(contact - cap_a);
					if (d < cyl_radius)
					{
						result.n1 = rc.normal; // normal of t1 is cap a normal
						max_t = MAX(max_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge case");
				}
			}
			if (!c2bx)
			{
				auto rc = raycast::plane(ray_origin, ray_dir, cap_b, -cyl_dir);
				if (rc.result)
				{
					vec3 contact = ray_origin + ray_dir * rc.t0;
					real d = math::length(contact - cap_b);
					if (d < cyl_radius)
					{
						result.n1 = rc.normal; // normal of t1 is cap b normal
						max_t = MAX(max_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge case");
				}
			}
		}

		result.result = min_t != INFINITY;
		result.t0 = min_t;
		result.t1 = max_t;
		// TODO: handle normals

		return result;
	}
#endif
}