
#include "nightpch.h"
#include "Quad.h"
#include "aabb/aabb.h"

namespace night
{
	Quad::Quad()
	{
		vertices[0].point = vec4(-1, 1, 0, 1);
		vertices[1].point = vec4(1, 1, 0, 1);
		vertices[2].point = vec4(1, -1, 0, 1);
		vertices[3].point = vec4(-1, -1, 0, 1);

		vertices[0].color = WHITE;
		vertices[1].color = WHITE;
		vertices[2].color = WHITE;
		vertices[3].color = WHITE;

		vertices[0].texture_coord = vec2{ 0.0f, 1.0f };
		vertices[1].texture_coord = vec2{ 1.0f, 1.0f };
		vertices[2].texture_coord = vec2{ 1.0f, 0.0f };
		vertices[3].texture_coord = vec2{ 0.0f, 0.0f };
	}

	Quad::Quad(const QuadParams& params)
	{
		vec3 p0 = vec3(-params.size.x, params.size.y, 0) + params.position; // top left
		vec3 p1 = vec3(params.size.x, params.size.y, 0) + params.position; // top right
		vec3 p2 = vec3(params.size.x, -params.size.y, 0) + params.position; // bottom right
		vec3 p3 = vec3(-params.size.x, -params.size.y, 0) + params.position; // bottom left

		vertices[0].point = vec4(p0, 1.0f);
		vertices[1].point = vec4(p1, 1.0f);
		vertices[2].point = vec4(p2, 1.0f);
		vertices[3].point = vec4(p3, 1.0f);

		vertices[0].color = params.color;
		vertices[1].color = params.color;
		vertices[2].color = params.color;
		vertices[3].color = params.color;

		vertices[0].texture_coord = params.texture_coords[0];
		vertices[1].texture_coord = params.texture_coords[1];
		vertices[2].texture_coord = params.texture_coords[2];
		vertices[3].texture_coord = params.texture_coords[3];
	}

	Quad::Quad(const AABB& aabb, const QuadParams& params)
	{
		vec3 p0 = vec3(aabb.left, aabb.top, 0) + params.position;
		vec3 p1 = vec3(aabb.right, aabb.top, 0) + params.position;
		vec3 p2 = vec3(aabb.right, aabb.bottom, 0) + params.position;
		vec3 p3 = vec3(aabb.left, aabb.bottom, 0) + params.position;

		vertices[0].point = vec4(p0, 1.0f); // TODO: rotate
		vertices[1].point = vec4(p1, 1.0f);
		vertices[2].point = vec4(p2, 1.0f);
		vertices[3].point = vec4(p3, 1.0f);

		vertices[0].color = params.color;
		vertices[1].color = params.color;
		vertices[2].color = params.color;
		vertices[3].color = params.color;

		vertices[0].texture_coord = params.texture_coords[0];
		vertices[1].texture_coord = params.texture_coords[1];
		vertices[2].texture_coord = params.texture_coords[2];
		vertices[3].texture_coord = params.texture_coords[3];
	}

	vec2 Quad::normalized_coordinate(vec3 const& point_on_quad) const
	{
		vec2 result;

		// top left to top right
		vec3 pptlx = project_point_to_line(point_on_quad, vertices[0].point, vec3(vertices[1].point)); // TODO: probably don't need to do this.
		result.x = dot((vec3(vertices[1].point) - vec3(vertices[0].point)) / length(vec3(vertices[1].point) - vec3(vertices[0].point)), (pptlx - vec3(vertices[0].point)) / length(vec3(vertices[1].point) - vec3(vertices[0].point))) * 2 - 1;

		// top left to bottom left
		vec3 pptly = project_point_to_line(point_on_quad, vertices[0].point, vec3(vertices[3].point));
		result.y = -(dot((vec3(vertices[3].point) - vec3(vertices[0].point)) / length(vec3(vertices[3].point) - vec3(vertices[0].point)), (pptly - vec3(vertices[0].point)) / length(vec3(vertices[3].point) - vec3(vertices[0].point))) * 2 - 1);

		return result;
	}

}