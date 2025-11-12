#pragma once

#include "core.h"
#include "Vertex.h"
#include "math/math.h"
#include "aabb/aabb.h"
#include "raycast/raycast.h"

namespace night
{
	//struct AABB;

	template<typename T = real>
	struct QuadParams
	{
		vec<3, T> position{ 0.0f, 0.0f, 0.0f };
		vec<2, T> size{ 1.0f, 1.0f };
		Color color{ WHITE };
		array<vec<2, r32>, 4> texture_coords
		{
			// coords must match renderer up vector
			vec<2, r32>{ 0.0f, 1.0f }, // top left
			vec<2, r32>{ 1.0f, 1.0f }, // top right
			vec<2, r32>{ 1.0f, 0.0f }, // bottom right
			vec<2, r32>{ 0.0f, 0.0f }, // bottom left
		};
	};

	template<typename T = real>
	struct Quad
	{
		array<Vertex<T>, 4> vertices;

		Quad();
		Quad(const QuadParams<T>& params);
		Quad(const AABB<T>& aabb, const QuadParams<T>& params = {});

		vec<2, T> normalized_coordinate(vec<3, T> const& point_on_quad) const;
	};

	template<typename T>
	Quad<T>::Quad()
	{
		vertices[0].point = vec<4, T>(-1, 1, 0, 1);
		vertices[1].point = vec<4, T>(1, 1, 0, 1);
		vertices[2].point = vec<4, T>(1, -1, 0, 1);
		vertices[3].point = vec<4, T>(-1, -1, 0, 1);

		vertices[0].color = WHITE;
		vertices[1].color = WHITE;
		vertices[2].color = WHITE;
		vertices[3].color = WHITE;

		vertices[0].texture_coord = vec2{ 0.0f, 1.0f };
		vertices[1].texture_coord = vec2{ 1.0f, 1.0f };
		vertices[2].texture_coord = vec2{ 1.0f, 0.0f };
		vertices[3].texture_coord = vec2{ 0.0f, 0.0f };
	}

	template<typename T>
	Quad<T>::Quad(const QuadParams<T>& params)
	{
		vec<3, T> p0 = vec<3, T>(-params.size.x, params.size.y, 0) + params.position; // top left
		vec<3, T> p1 = vec<3, T>(params.size.x, params.size.y, 0) + params.position; // top right
		vec<3, T> p2 = vec<3, T>(params.size.x, -params.size.y, 0) + params.position; // bottom right
		vec<3, T> p3 = vec<3, T>(-params.size.x, -params.size.y, 0) + params.position; // bottom left

		vertices[0].point = vec<4, T>(p0, 1.0f);
		vertices[1].point = vec<4, T>(p1, 1.0f);
		vertices[2].point = vec<4, T>(p2, 1.0f);
		vertices[3].point = vec<4, T>(p3, 1.0f);

		vertices[0].color = params.color;
		vertices[1].color = params.color;
		vertices[2].color = params.color;
		vertices[3].color = params.color;

		vertices[0].texture_coord = params.texture_coords[0];
		vertices[1].texture_coord = params.texture_coords[1];
		vertices[2].texture_coord = params.texture_coords[2];
		vertices[3].texture_coord = params.texture_coords[3];
	}

	template<typename T>
	Quad<T>::Quad(const AABB<T>& aabb, const QuadParams<T>& params)
	{
		vec<3, T> p0 = vec<3, T>{ aabb.left, aabb.top, 0 } + params.position;
		vec<3, T> p1 = vec<3, T>{ aabb.right, aabb.top, 0 } + params.position;
		vec<3, T> p2 = vec<3, T>{ aabb.right, aabb.bottom, 0 } + params.position;
		vec<3, T> p3 = vec<3, T>{ aabb.left, aabb.bottom, 0 } + params.position;

		vertices[0].point = vec<4, T>{ p0, 1.0f }; // TODO: rotate
		vertices[1].point = vec<4, T>{ p1, 1.0f };
		vertices[2].point = vec<4, T>{ p2, 1.0f };
		vertices[3].point = vec<4, T>{ p3, 1.0f };

		vertices[0].color = params.color;
		vertices[1].color = params.color;
		vertices[2].color = params.color;
		vertices[3].color = params.color;

		vertices[0].texture_coord = params.texture_coords[0];
		vertices[1].texture_coord = params.texture_coords[1];
		vertices[2].texture_coord = params.texture_coords[2];
		vertices[3].texture_coord = params.texture_coords[3];
	}

	template<typename T>
	vec<2, T> Quad<T>::normalized_coordinate(vec<3, T> const& point_on_quad) const
	{
		// TODO: this function is bad
		vec<2, T> result;

		// top left to top right
		vec<3, T> pptlx = Raycast3D<T>::project_point_to_ray(point_on_quad, vertices[0].point, vec<3, T>(vertices[1].point) - vec<3, T>(vertices[0].point)); // TODO: probably don't need to do this.
		result.x = Math<T>::dot((vec<3, T>(vertices[1].point) - vec<3, T>(vertices[0].point)) / Math<T>::length(vec<3, T>(vertices[1].point) - vec<3, T>(vertices[0].point)), (pptlx - vec<3, T>(vertices[0].point)) / Math<T>::length(vec<3, T>(vertices[1].point) - vec<3, T>(vertices[0].point))) * 2 - 1;

		// top left to bottom left
		vec<3, T> pptly = Raycast3D<T>::project_point_to_ray(point_on_quad, vertices[0].point, vec<3, T>(vertices[3].point) - vec<3, T>(vertices[0].point));
		result.y = -(Math<T>::dot((vec<3, T>(vertices[3].point) - vec<3, T>(vertices[0].point)) / Math<T>::length(vec<3, T>(vertices[3].point) - vec<3, T>(vertices[0].point)), (pptly - vec<3, T>(vertices[0].point)) / Math<T>::length(vec<3, T>(vertices[3].point) - vec<3, T>(vertices[0].point))) * 2 - 1);

		return result;
	}
}

