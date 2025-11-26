#pragma once

#include "math/Math.h"
#include "log/log.h"
#include "convex_hull.h"
#include "raycast/Raycast.h"
#include "gjk/GJK.h"

namespace night
{

	template<typename T = real>
	struct NIGHT_API Convex2D
	{
		template<typename _It>
		static vector<vec<2, T>> make_convex(_It begin, _It end);

		template<typename _It>
		static b8 is_clockwise(_It begin, _It end);

		template<typename _It>
		static void wind_up(_It begin, _It end, EOrientation winding_order);

		// TODO: template raycast
		template<typename _It>
		static typename RaycastResult2D<T> raycast(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, _It begin, _It end);

		template<typename _It>
		static T area(_It begin, _It end);

		//// TODO: may remove
		//template<typename _It>
		//static T arc_length(_It begin, _It end);

		template<typename _Itc, typename _Its>
		static vector<vec<2, T>> clip(_Itc begin_clip, _Itc end_clip, _Its begin_subject, _Its end_subject);

		template<typename _It>
		static b8 intersects(vec<2, T> const& point, _It begin, _It end, real epsilon = NIGHT_GJK_DEFAULT_EPSILON, s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS);
	};

	template<typename T>
	template<typename _It>
	static vector<vec<2, T>> Convex2D<T>::make_convex(_It begin, _It end)
	{
		vector<vec<2, T>> point_cloud;
		for (auto i = begin; i != end; i++) point_cloud.push_back((vec<2, T> const&)(*i));
		// TODO: this function is not templated, need to properly implement it.
		return place_holder::quickHull(point_cloud);
	}

	template<typename T>
	template<typename _It>
	b8 Convex2D<T>::is_clockwise(_It begin, _It end)
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

#if 0
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
#endif

	//constexpr real _cross(vec2 a, vec2 b) { return a.x * b.y - b.x * a.y; }

	//constexpr vec2 _intersection(vec2 a1, vec2 a2, vec2 b1, vec2 b2) {
	//	return ((b1 - b2) * math::cross(a1, a2) - (a1 - a2) * math::cross(b1, b2)) *
	//		(1.0f / math::cross(a1 - a2, b1 - b2));
	//}

	template<typename T>
	template<typename _Itc, typename _Its>
	inline vector<vec<2, T>> Convex2D<T>::clip(_Itc begin_clip, _Itc end_clip, _Its begin_subject, _Its end_subject)
	{
		//DB_ALGO_SCOPED("__area_of_intersection");
		//constexpr real __db_mult = 6.0f;

		if (begin_clip == end_clip || begin_subject == end_subject)
		{
			return {};
		}

		auto is_inside = [](vec2 point, vec2 a, vec2 b) -> b8
			{
				return (math::cross(a - b, point) + math::cross(b, a)) > 0.0f;
			};

		vector<vec2> clip_polygon;
		vector<vec2> ring;
		vector<vec2> input;

		for (_Itc i = begin_clip; i != end_clip; i++)
		{
			clip_polygon.push_back((vec2 const&)(*i));
		}

		for (_Its i = begin_subject; i != end_subject; i++)
		{
			ring.push_back((vec2 const&)(*i));
		}

		vec2 p1 = clip_polygon[clip_polygon.size() - 1];

		for (vec2 p2 : clip_polygon)
		{
			if (ring.empty())
			{
				return {};
			}

//			DB_ALGO_INCREMENT_STEP();
//#ifdef NIGHT_DBAR
//			for (s32 i = 0; i < ring.size(); i++)
//			{
//				vec2 const& p1 = ring[i];
//				vec2 const& p2 = ring[(i + 1) % ring.size()];
//				DB_ALGO_DRAW_LINE(p1 * __db_mult, p2 * __db_mult, PINK.opaqued(0.75f));
//			}
//#endif

			//DB_ALGO_DRAW_LINE(p1 * __db_mult, p2 * __db_mult, RED);

			input.clear();
			input.insert(input.end(), ring.begin(), ring.end());
			ring.clear();

			{
				//DB_ALGO_SCOPED("inner loop");

				vec2 s = input[input.size() - 1];

				for (vec2 e : input)
				{
					//DB_ALGO_INCREMENT_STEP();
					//DB_ALGO_DRAW_LINE(s * __db_mult, e * __db_mult, BLUE);

					if (is_inside(e, p1, p2))
					{
						if (!is_inside(s, p1, p2))
						{
							//DB_ALGO_DRAW_POINT(_intersection(p1, p2, s, e) * __db_mult, YELLOW.opaqued(0.5f));

							auto rc = Raycast2D<>::plane(p1, p2 - p1, s, math::perp(e - s), NIGHT_EPSILON_MEDIUM);

							if (rc.result)
							{
								ring.push_back(rc.contact(p1, p2 - p1));
							}

							//if (abs(_cross(p2 - p1, e - s)) > NIGHT_EPSILON_MEDIUM)
							//{
							//	ring.push_back(_intersection(p1, p2, s, e));
							//}
						}

						//DB_ALGO_DRAW_POINT(e * __db_mult, CYAN);
						ring.push_back(e);
					}
					else if (is_inside(s, p1, p2))
					{
						//DB_ALGO_DRAW_POINT(_intersection(p1, p2, s, e) * __db_mult, YELLOW);

						//if (abs(_cross(p2 - p1, e - s)) > NIGHT_EPSILON_MEDIUM)
						//{
						//	ring.push_back(_intersection(p1, p2, s, e));
						//}

						auto rc = Raycast2D<>::plane(p1, p2 - p1, s, math::perp(e - s), NIGHT_EPSILON_MEDIUM);

						if (rc.result)
						{
							ring.push_back(rc.contact(p1, p2 - p1));
						}
					}

					s = e;
				}
			}

			p1 = p2;
		}

//		DB_ALGO_INCREMENT_STEP();
//#ifdef NIGHT_DBAR
//		for (s32 i = 0; i < ring.size(); i++)
//		{
//			vec2 const& p1 = ring[i];
//			vec2 const& p2 = ring[(i + 1) % ring.size()];
//			DB_ALGO_DRAW_LINE(p1 * __db_mult, p2 * __db_mult, GREEN.opaqued(0.75f));
//		}
//#endif

		return ring;
	}

	template<typename T>
	template<typename _It>
	inline b8 Convex2D<T>::intersects(vec<2, T> const& point, _It begin, _It end, real epsilon, s32 max_iterations)
	{
#if 1 // TODO: use gjk
		if (begin == end || std::next(begin) == end)
		{
			return false;
		}

		b8 clockwise = Convex2D<T>::is_clockwise(begin, end);

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
	inline void Convex2D<T>::wind_up(_It begin, _It end, EOrientation winding_order)
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

			if (winding_order == EOrientation::Clockwise)
			{
				angle.angle = Math<T>::angle_counter_clockwise(vec<2, T>(0, 1), Math<T>::normalize((vec<2, T> const&)angle.element - center));
			}
			else if (winding_order == EOrientation::CounterClockwise)
			{
				angle.angle = Math<T>::angle_clockwise(vec<2, T>(0, 1), Math<T>::normalize((vec<2, T> const&)angle.element - center));
			}
			else
			{
				ASSERT(false); // winding order is invalid
			}
			
			angle.iterator = i;

			angles.push_back(angle);
		}

		std::sort(angles.begin(), angles.end(), [](auto& a, auto& b) -> b8 {return a.angle < b.angle; });

		for (s32 i = 0; i < angles.size(); i++)
		{
			(*(begin + i)) = angles[i].element;
		}
	}

	//using convex2d = Convex2D<real>;
	//using dconvex2d = Convex2D<r64>;
	//using fconvex2d = Convex2D<r32>;
}