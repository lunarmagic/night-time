#pragma once

#include "core.h"
#include "math/Math.h"

namespace night
{

	template<typename T>
	struct Quad;

	template<typename T = real>
	struct AABB
	{
		T left{ INFINITY };
		T right{ -INFINITY };
		T top{ -INFINITY }; // TODO up is +.
		T bottom{ INFINITY };

		void fit_around_point(const vec<2, T>& point);
		void fit_around_quad(Quad<T> const& quad);
		u8 contains(vec<2, T> const& point);
		T width() const { return right - left; }
		T height() const { return top - bottom; }

		vec<2, T> local_coordinate(vec<2, T> const& global_coordinate);
	};

	template<typename T>
	u8 AABB<T>::contains(vec<2, T> const& point)
	{
		return (point.x > left && point.x < right && point.y < top && point.y > bottom);
	}

	template<typename T>
	vec<2, T> AABB<T>::local_coordinate(vec<2, T> const& global_coordinate)
	{
		// TODO: this function only works with reals
		T x = ((global_coordinate.x - left) / width()) * 2.0f - 1.0f;
		T y = ((global_coordinate.y - bottom) / height()) * 2.0f - 1.0f;

		return
		{
			 x,
			 y
		};
	}

	template<typename T>
	void AABB<T>::fit_around_point(vec<2, T> const& point)
	{
		left = MIN(point.x, left);
		right = MAX(point.x, right);
		top = MAX(point.y, top);
		bottom = MIN(point.y, bottom);
	}

	template<typename T>
	void AABB<T>::fit_around_quad(Quad<T> const& quad)
	{
		for (const auto& i : quad.vertices)
		{
			left = MIN(left, i.point.x);
			right = MAX(right, i.point.x);
			top = MAX(top, i.point.y);
			bottom = MIN(bottom, i.point.y);
		}
	}

}
