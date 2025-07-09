
//#include "sandboxpch.h"
#include "nightpch.h"
#include "Plane.h"
#include "log/log.h"

namespace night
{

	raycast::Result3D Plane::raycast(vec3 const& ray_origin, vec3 const& ray_direction) const
	{
		ASSERT(vertices.size() >= 3); // must be at least a triangle

		auto rc = raycast::plane(ray_origin, ray_direction, vertices[0], normal);
		vec3 contact = rc.contact(ray_origin, ray_direction);

		for (s32 i = 0; i < vertices.size(); i++)
		{
			vec3 const& p1 = vertices[i];
			vec3 const& p2 = vertices[(i + 1) % vertices.size()];
			vec3 const& p3 = vertices[(i + 2) % vertices.size()];
			vec3 c3 = triple_cross(p2 - p1, p3 - p1, p2 - p1);
			real d = distance_to_plane(contact, p1, c3);

			if (d < 0.0f)
			{
				return { .result = false };
			}
		}

		return rc;
	}

}