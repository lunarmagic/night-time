#pragma once

#include "core.h"
#include "Vertex.h"
#include "math/math.h"
//#include "stl/stl.h"
//#include "texture/ITexture.h"

namespace night
{

	struct AABB;

	struct QuadParams
	{
		vec3 position{ 0.0f, 0.0f, 0.0f };
		vec2 size{ 1.0f, 1.0f };
		// TODO: rotation
		Color color{ WHITE };
		array<vec2, 4> texture_coords
		{
			// coords must match renderer up vector
			vec2{ 0.0f, 1.0f }, // top left
			vec2{ 1.0f, 1.0f }, // top right
			vec2{ 1.0f, 0.0f }, // bottom right
			vec2{ 0.0f, 0.0f }, // bottom left
		};
	};

	struct NIGHT_API Quad
	{
		array<Vertex, 4> vertices;

		Quad();
		Quad(const QuadParams& params);
		Quad(const AABB& aabb, const QuadParams& params = {});

		vec2 normalized_coordinate(vec3 const& point_on_quad) const;
	};

}

