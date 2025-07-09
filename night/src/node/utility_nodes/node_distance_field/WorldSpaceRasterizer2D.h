#pragma once

#include "geometry/Quad.h"
#include "log/log.h"

namespace night
{
	template<typename T>
	struct LineFragmentData
	{
		T* fragment;
		ivec2 coordinate;
		real t;
	};

	template<typename T>
	struct WorldSpaceRasterizer2D
	{
		WorldSpaceRasterizer2D() = default;
		WorldSpaceRasterizer2D(T* fragments, s32 width, s32 height, Quad const& area = Quad(QuadParams{}));

		// TODO: return vec3
		vec2 global_to_local(const vec2& global_coordinate) const;
		ivec2 local_to_internal(const vec2& local_coordinate) const;
		vec3 internal_to_world(const ivec2& internal_coordinate) const;

		u8 is_coord_in_bounds(const ivec2& coordinate) const;

		void area(Quad const& a) { _area = a; }
		void fragments(T* f) { _fragments = f; };
		void width(s32 w) { _width = w; }
		void height(s32 h) { _height = h; }

		// vertex zero should be top-left, winding order is clockwise
		Quad const& area() const { return _area; }

		T* fragments() const { return _fragments; };
		s32 width() const { return  _width; }
		s32 height() const { return  _height; }

		void fill(const T& fragment);

		void draw_line(ivec2 const& p1, ivec2 const& p2, function<void(const LineFragmentData<T>&)> callback, u8 should_draw_out_of_bounds = false) const;

		void draw_line(vec2 const& p1, vec2 const& p2, function<void(const LineFragmentData<T>&)> callback, u8 should_draw_out_of_bounds = false) const;


	private:

		Quad _area{ QuadParams{} };
		T* _fragments = nullptr;
		s32 _width = 0;
		s32 _height = 0;
	};

	template<typename T>
	inline WorldSpaceRasterizer2D<T>::WorldSpaceRasterizer2D(T* fragments, s32 width, s32 height, Quad const& area)
		: _fragments(fragments)
		, _width(width)
		, _height(height)
		, _area(area)
	{
	}

	template<typename T>
	inline vec2 WorldSpaceRasterizer2D<T>::global_to_local(const vec2& global_coordinate) const
	{
		real min_x = INFINITY; // TODO: rotate vertices
		real max_x = -INFINITY;
		real min_y = INFINITY;
		real max_y = -INFINITY;

		for (s32 i = 0; i < 4; i++)
		{
			auto& vertex = _area.vertices[i];
			min_x = MIN(vertex.point.x, min_x);
			max_x = MAX(vertex.point.x, max_x);
			min_y = MIN(vertex.point.y, min_y);
			max_y = MAX(vertex.point.y, max_y);
		}

		vec2 result;
		result.x = ((global_coordinate.x - min_x) / abs(max_x - min_x)) * 2.0f - 1.0f;
		result.y = ((global_coordinate.y - min_y) / abs(max_y - min_y)) * 2.0f - 1.0f;

		return result;
	}

	template<typename T>
	inline ivec2 WorldSpaceRasterizer2D<T>::local_to_internal(const vec2& local_coordinate) const
	{
		return
		{
			(local_coordinate.x + 1.0f) * (_width / 2),
			(local_coordinate.y + 1.0f) * (_height / 2)
		};
	}

	template<typename T>
	inline vec3 WorldSpaceRasterizer2D<T>::internal_to_world(const ivec2& internal_coordinate) const
	{
		vec2 t;
		t.x = (real)internal_coordinate.x / width();
		t.y = (real)internal_coordinate.y / height();

		vec3 top_left = _area.vertices[0].point;
		vec3 top_right = _area.vertices[1].point;
		vec3 bottom_right = _area.vertices[2].point;
		vec3 bottom_left = _area.vertices[3].point;

		vec3 result;
		result = bottom_left;
		result += (bottom_right - bottom_left) * t.x;
		result += (top_left - bottom_left) * t.y; // TODO: support non-rectangles
		return result;
	}

	template<typename T>
	inline u8 WorldSpaceRasterizer2D<T>::is_coord_in_bounds(const ivec2& coordinate) const
	{
		return coordinate.x >= 0 && coordinate.x < _width && coordinate.y >= 0 && coordinate.y < _height;
	}

	template<typename T>
	inline void WorldSpaceRasterizer2D<T>::fill(const T& fragment)
	{
		ASSERT(_fragments != nullptr); // set fragments before drawing.
		ASSERT(_width != 0 && _height != 0); // set width and height

		for (s32 y = 0; y < _height; y++)
		{
			for (s32 x = 0; x < _width; x++)
			{
				_fragments[x + y * _width] = fragment;
			}
		}
	}

	template<typename T>
	inline void WorldSpaceRasterizer2D<T>::draw_line(ivec2 const& p1, ivec2 const& p2, function<void(const LineFragmentData<T>&)> callback, u8 should_draw_out_of_bounds) const
	{
		ASSERT(_fragments != nullptr); // set fragments before drawing.
		ASSERT(_width != 0 && _height != 0); // set width and height
		ASSERT(callback != nullptr); // callback is nullptr

		s32 dx = p2.x - p1.x;
		s32 dy = p2.y - p1.y;

		s32 d_long = abs(dx);
		s32 d_short = abs(dy);

		ivec2 offset_long = dx > 0 ? ivec2{ 1, 0 } : ivec2{ -1, 0 };
		ivec2 offset_short = dy > 0 ? ivec2{ 0, 1 } : ivec2{ 0, -1 };

		if (d_long < d_short)
		{
			SWAP(d_long, d_short);
			SWAP(offset_short, offset_long);
		}

		ivec2 current = p1;

		s32 error = (d_short * 2) - d_long;
		constexpr s32 max_iterations = 1000; // TODO: handle this correctly.
		for (s32 i = 0; i < d_long && i < max_iterations; i++)
		{
			u8 in_bounds = is_coord_in_bounds(current);

			if (!in_bounds && !should_draw_out_of_bounds)
			{
				continue;
			}

			s32 index = current.x + current.y * _width;
			real t = (real)i / (real)(d_long - 1);

			callback(
				{
				.fragment = in_bounds ? &_fragments[index] : nullptr,
				.coordinate = current,
				.t = t,
				}
			); // TODO: impl t

			if (error >= 0)
			{
				current += offset_long + offset_short;
				error += (d_short - d_long) * 2;
			}
			else
			{
				current += offset_long;
				error += d_short * 2;
			}
		}
	}

	template<typename T>
	inline void WorldSpaceRasterizer2D<T>::draw_line(vec2 const& p1, vec2 const& p2, function<void(const LineFragmentData<T>&)> callback, u8 should_draw_out_of_bounds) const
	{
		ivec2 p1i = local_to_internal(global_to_local(p1));
		ivec2 p2i = local_to_internal(global_to_local(p2));

		draw_line(p1i, p2i, callback, should_draw_out_of_bounds);
	}

}