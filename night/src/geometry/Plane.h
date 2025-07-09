#pragma once

#include "raycast/raycast.h"
#include "debug_renderer/DebugRenderer.h"

namespace night
{
	struct NIGHT_API Plane
	{
		vector<vec3> vertices;
		vec3 normal;

		raycast::Result3D raycast(vec3 const& ray_origin, vec3 const& ray_direction) const;
	};

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
}