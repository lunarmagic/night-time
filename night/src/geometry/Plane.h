#pragma once

#include "raycast/raycast.h"
#include "debug_renderer/DebugRenderer.h"

namespace night
{

	template<typename T = real>
	struct Plane
	{
		vector<vec<3, T>> vertices;
		vec<3, T> normal;

		RaycastResult3D<T> raycast(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction) const;
	};

	template<typename T>
	RaycastResult3D<T> Plane<T>::raycast(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction) const
	{
		ASSERT(vertices.size() >= 3); // must be at least a triangle

		auto rc = Raycast3D<T>::plane(ray_origin, ray_direction, vertices[0], normal);
		vec<3, T> contact = rc.contact(ray_origin, ray_direction);

		for (s32 i = 0; i < vertices.size(); i++)
		{
			vec<3, T> const& p1 = vertices[i];
			vec<3, T> const& p2 = vertices[(i + 1) % vertices.size()];
			vec<3, T> const& p3 = vertices[(i + 2) % vertices.size()];
			vec<3, T> c3 = Math<T>::triple_cross(p2 - p1, p3 - p1, p2 - p1);
			T d = Math<T>::distance_to_plane(contact, p1, c3);

			if (d < 0)
			{
				return { .result = false };
			}
		}

		return rc;
	}

#if 0
#ifdef NIGHT_ENABLE_DEBUG_RENDERER
	template<> inline void DebugRenderer::draw_format<Plane>(Plane& v)
	{
		vec3 center = ORIGIN;
		s32 i;
		for (i = 0; i < v.vertices.size(); i++)
		{
			vec3 p1 = v.vertices[i];
			vec3 p2 = v.vertices[(i + 1) % v.vertices.size()];
			DB_RENDERER_DRAW_LINE(p1, p2, ORANGE);
			center += p1;
		}
		center /= (real)i;
		DebugRenderer::draw_arrow({ .origin = center, .direction = v.normal, .color = RED });
	}

	template<> inline void DebugRenderer::draw_format<vector<Plane>>(vector<Plane>& v)
	{
		for (const auto& i : v)
		{
			DB_RENDERER_DRAW_OBJECT(i);
		}
	}
#endif
#endif
}