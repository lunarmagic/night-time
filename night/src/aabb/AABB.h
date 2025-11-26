#pragma once

#include "core.h"
#include "math/Math.h"

namespace night
{

	template<typename T>
	struct Quad;

	struct STRCT
	{
		string s;
	};

	template<typename T = real>
	struct AABB
	{
		T left{ numeric_limits<T>::max() };
		T top{ numeric_limits<T>::lowest() };
		T right{ numeric_limits<T>::lowest() };
		T bottom{ numeric_limits<T>::max() };

		vec<2, T> top_left() const { return { left, top }; }
		void top_left(vec<2, T> const& vec) { left = vec.x, top = vec.y; }

		vec<2, T> bottom_right() const { return { right, bottom }; }
		void bottom_right(vec<2, T> const& vec) { right = vec.x, bottom = vec.y; }

		void fit_around_point(const vec<2, T>& point);
		void fit_around_quad(Quad<T> const& quad);
		b8 contains(vec<2, T> const& point);
		T width() const { return right - left; }
		T height() const { return top - bottom; }

		vec<2, T> local_coordinate(vec<2, T> const& global_coordinate);

		static AABB<T> combine(AABB<T> const& a, AABB<T> const& b)
		{
			AABB<T> result;
			result.left = MIN(a.left, b.left);
			result.right = MAX(a.right, b.right);
			result.top = MAX(a.top, b.top);
			result.bottom = MIN(a.bottom, b.bottom);
			return result;
		}
	};

	template<typename T>
	b8 AABB<T>::contains(vec<2, T> const& point)
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

	template<typename T>
	static inline string _paabb(AABB<T> aabb)
	{
		sstream stream;
		stream << "{ " << aabb.left << ", " << aabb.right <<  ", " <<  aabb.top << ", " << aabb.bottom << "}";
		return stream.str();
	}

	template<> inline string Log::print_format<AABB<s8>>(AABB<s8>& v) { return _paabb(v); }
	template<> inline string Log::print_format<AABB<b8>>(AABB<b8>& v) { return _paabb(v); }

	template<> inline string Log::print_format<AABB<s16>>(AABB<s16>& v) { return _paabb(v); }
	template<> inline string Log::print_format<AABB<u16>>(AABB<u16>& v) { return _paabb(v); }

	template<> inline string Log::print_format<AABB<r32>>(AABB<r32>& v) { return _paabb(v); }
	template<> inline string Log::print_format<AABB<s32>>(AABB<s32>& v) { return _paabb(v); }
	template<> inline string Log::print_format<AABB<u32>>(AABB<u32>& v) { return _paabb(v); }

	template<> inline string Log::print_format<AABB<r64>>(AABB<r64>& v) { return _paabb(v); }
	template<> inline string Log::print_format<AABB<s64>>(AABB<s64>& v) { return _paabb(v); }
	template<> inline string Log::print_format<AABB<u64>>(AABB<u64>& v) { return _paabb(v); }

}
