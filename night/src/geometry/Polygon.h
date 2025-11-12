#pragma once

#include "math/math.h"
#include "log/log.h"
#include "convex_hull.h"
#include "raycast/raycast.h"
#include "gjk/gjk.h"

namespace night
{

	template<typename T = real>
	struct NIGHT_API Convex2D
	{
		template<typename _It>
		static u8 is_clockwise(_It begin, _It end);

		template<typename _It>
		static void make_clockwise(_It begin, _It end);

		// TODO: template raycast
		template<typename _It>
		static typename RaycastResult2D<T> raycast(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, _It begin, _It end);

		template<typename _It>
		static T area(_It begin, _It end);

		// TODO: may remove
		template<typename _It>
		static T arc_length(_It begin, _It end);

		template<typename _It>
		static u8 intersects(vec<2, T> const& point, _It begin, _It end, real epsilon = NIGHT_GJK_DEFAULT_EPSILON, s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS);
	};

	template<typename T>
	template<typename _It>
	u8 Convex2D<T>::is_clockwise(_It begin, _It end)
	{
		// TODO: this function seems to be broken.

		if (begin == end)
		{
			return false;
		}

		if (std::distance(begin, end) < 3)
		{
			return true; // convex must be atleast a triangle.
		}

		for (auto i = begin; i != end; i++)
		{
			vec<2, T> const& p1 = (vec<2, T> const&)(*i);
			auto j = std::next(i);
			if (j == end) j = begin;
			vec<2, T> const& p2 = (vec<2, T> const&)(*j);
			auto k = std::next(j);
			if (k == end) k = begin;
			vec<2, T> const& p3 = (vec<2, T> const&)(*k);

			EOrientation o = Math<T>::orientation(p1, p2, p3);

			if (o == EOrientation::CounterClockwise)
			{
				return false;
			}
		}

		return true;
	}

	template<typename T>
	template<typename _It>
	inline RaycastResult2D<T> Convex2D<T>::raycast(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, _It begin, _It end)
	{
#if 1
		if (begin == end || std::next(begin) == end)
		{
			return typename Raycast2D<T>::Result{ .result = false };
		}

		typename Raycast2D<T>::Result result;

		T max_t = -INFINITY;
		T min_t = INFINITY;

		vec<2, T> max_normal = {};
		vec<2, T> min_normal = {};

		auto i = std::prev(end);

		for (auto j = begin; j != end; j++)
		{
			typename Raycast2D<T>::Result rc = Raycast2D<T>::line(ray_origin, ray_direction, (vec<2, T> const&)(*i), (vec<2, T> const&)(*j));

			if (rc.result)
			{
				if (rc.t > max_t)
				{
					max_t = rc.t;
					max_normal = rc.normal;
				}
				
				if (rc.t < min_t)
				{
					min_t = rc.t;
					min_normal = rc.normal;
				}
			}

			i = j;
		}

		result.result = (max_t != -INFINITY && min_t != INFINITY);
		result.t0 = min_t;
		result.t1 = max_t;
		result.n0 = min_normal;
		result.n1 = max_normal;
		return result;
#endif
	}

	template<typename T>
	template<typename _It>
	inline T Convex2D<T>::area(_It begin, _It end)
	{
		if (begin == end)
		{
			return 0.0f;
		}

		T area = 0.0f;

		_It i = std::prev(end);
		for (_It j = begin; j != end; j++)
		{
			vec<2, T> const& p1 = (vec<2, T> const&)(*i);
			vec<2, T> const& p2 = (vec<2, T> const&)(*j);
			area += 0.5f * (p1.x * p2.y - p2.x * p1.y);
			i = j;
		}

		return abs(area);
	}

	template<typename T>
	template<typename _It>
	inline T Convex2D<T>::arc_length(_It begin, _It end)
	{
		return Math<T>::arc_length(begin, end, true);
#if 0
		if (begin == end || std::next(begin) == end)
		{
			return 0.0f;
		}

		real result = 0.0f;

#if 1
		_It i = std::prev(end);
		for (_It j = begin; j != end; j++)
		{
			result += Math<T>::length((vec2 const&)(*j) - (vec2 const&)(*i));
			i = j;
		}
#else
		for (auto i = begin; i != end; i++)
		{
			vec2 const& p1 = (vec2 const&)(*i);
			auto j = std::next(i);
			if (j == end)
			{
				j = begin;
			}
			vec2 const& p2 = (vec2 const&)(*j);

			result += length(p2 - p1);
		}
#endif

		return result;
#endif
	}

	template<typename T>
	template<typename _It>
	inline u8 Convex2D<T>::intersects(vec<2, T> const& point, _It begin, _It end, real epsilon, s32 max_iterations)
	{
#if 1 // TODO: use gjk
		if (begin == end || std::next(begin) == end)
		{
			return false;
		}

		u8 clockwise = Convex2D<T>::is_clockwise(begin, end);

		_It i = std::prev(end);
		for (_It j = begin; j != end; j++)
		{
			if (Math<T>::orientation((vec<2, T> const&)(*i), (vec<2, T> const&)(*j), point) == (clockwise ? EOrientation::CounterClockwise : EOrientation::Clockwise))
			{
				return false;
			}

			i = j;
		}

		return true;
#else
		auto support = [&](dvec2 const& direction) -> dvec2
			{
				dvec2 dir_normalized = Math<T>::normalize(direction); // TODO: remove
				dvec2 result = {};
				r64 max_dot = -INFINITY;

				for (_It i = begin; i != end; i++)
				{
					dvec2 point = (vec2 const&)(*i);
					r64 new_d = Math<T>::dot(dir_normalized, point);
					if (new_d > max_dot)
					{
						max_dot = new_d;
						result = point;
					};
				}

				return result;
			};

		return gjk::intersects([&](dvec2 const&) ->dvec2 { return point; }, support, (r64)epsilon, max_iterations);
#endif
	}

	template<typename T>
	template<typename _It>
	inline void Convex2D<T>::make_clockwise(_It begin, _It end)
	{
		if (begin == end)
		{
			// container is empty.
			return;
		}

		vec<2, T> center = vec<2, T>(0);
		u32 count = 0;

		for (auto i = begin; i != end; i++)
		{
			center += (vec<2, T> const&)(*i);
			count++;
		}

		if (count == 0)
		{
			WARNING("count is 0 / out_approx is empty");
			return;
		}

		center /= count;

		struct Angle
		{
			typename std::remove_pointer<decltype(&(*begin))>::type element;
			T angle;
			_It iterator;
		};

		vector<Angle> angles;

		for (auto i = begin; i != end; i++)
		{
			Angle angle;

			angle.element = (*i);
			angle.angle = Math<T>::angle_counter_clockwise(vec<2, T>(0, 1), Math<T>::normalize((vec<2, T> const&)angle.element - center));
			angle.iterator = i;

			angles.push_back(angle);
		}

		std::sort(angles.begin(), angles.end(), [](auto& a, auto& b) -> u8 {return a.angle < b.angle; });

		for (s32 i = 0; i < angles.size(); i++)
		{
			(*(begin + i)) = angles[i].element;
		}
	}

	//using convex2d = Convex2D<real>;
	//using dconvex2d = Convex2D<r64>;
	//using fconvex2d = Convex2D<r32>;
}