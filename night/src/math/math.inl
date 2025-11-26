#pragma once
#include "math.h"

namespace night
{

	template<typename T>
	inline T Math<T>::cross(vec<2, T> const& a, vec<2, T> const& b)
	{
		return a.x * b.y - b.x * a.y;
	}

	template<typename T>
	inline T Math<T>::lerp(T a, T b, T t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	inline vec<2, T> Math<T>::lerp(vec<2, T> a, vec<2, T> b, T t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	inline vec<3, T> Math<T>::lerp(vec<3, T> a, vec<3, T> b, T t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	inline T Math<T>::ilerp(T a, T b, T c)
	{
		return (c - a) / (b - a);
	}

	template<typename T>
	inline T Math<T>::ilerp(vec<2, T> a, vec<2, T> b, vec<2, T> c)
	{
		return length(c - a) / length(b - a);
	}

	template<typename T>
	inline T Math<T>::ilerp(vec<3, T> a, vec<3, T> b, vec<3, T> c)
	{
		return length(c - a) / length(b - a);
	}

	template<typename T>
	inline T Math<T>::dampen(T a, T b, T t, T delta)
	{
		t = 1.0f - pow(t, delta);
		return lerp(a, b, t);
	}

	template<typename T>
	inline T Math<T>::smoothstep(T a, T b, T t)
	{
		T v1 = t * t;
		T v2 = 1.0f - (1.0f - t) * (1.0f - t);
		return lerp(a, b, lerp(v1, v2, t));
	}

	template<typename T>
	inline vec<2, T> Math<T>::bezier(const vec<2, T>& a, const vec<2, T>& b, const vec<2, T>& c, T t)
	{
		const T x = (1.f - t) * (1.f - t) * a.x + 2.f * (1.f - t) * t * b.x + t * t * c.x;
		const T y = (1.f - t) * (1.f - t) * a.y + 2.f * (1.f - t) * t * b.y + t * t * c.y;

		return { x, y };
	}

	//template<typename T>
	//inline vec<2, T> Math<T>::slerp(const vec<2, T>& a, const vec<2, T>& b, T t)
	//{
	//	T d = dot(a, b);
	//	d = CLAMP(d, -1.0f, 1.0f);
	//	T theta = acos(d) * t;
	//	vec<2, T> relative = normalize(b - a * d);
	//	return (a * cos(theta)) + (relative * sin(theta));
	//}

	template<typename T>
	inline T Math<T>::angle_clockwise(const vec<2, T>& a, const vec<2, T>& b)
	{
		T d = a.x * b.x + a.y * b.y;
		T det = a.x * b.y - a.y * b.x;
		T angle = atan2(-det, -d) + R_PI;
		return angle;
	}

	template<typename T>
	inline T Math<T>::angle_counter_clockwise(const vec<2, T>& a, const vec<2, T>& b)
	{
		T d = a.x * b.x + -a.y * -b.y;
		T det = a.x * -b.y - -a.y * b.x;
		T angle = atan2(-det, -d) + R_PI;
		return angle;
	}

	template<typename T>
	inline ProjectPointToPlaneResult2D<T> Math<T>::project_point_to_plane(const vec<2, T>& point, const vec<2, T>& plane_origin, const vec<2, T>& plane_normal)
	{
		T d = -dot(plane_normal, plane_origin);
		T distance = dot(plane_normal, point) + d;
		return ProjectPointToPlaneResult2D{ .point = point - plane_normal * distance, .distance = distance };
	}

	template<typename T>
	inline vec<2, T> Math<T>::project_point_to_line(const vec<2, T>& point, const vec<2, T>& line_p1, const vec<2, T>& line_p2)
	{
		vec<2, T> ab = line_p2 - line_p1;
		T t = dot(point - line_p1, ab) / dot(ab, ab);
		t = CLAMP(t, 0.0f, 1.0f);
		return line_p1 + ab * t;
	}

	template<typename T>
	inline ProjectPointToPlaneResult3D<T> Math<T>::project_point_to_plane(const vec<3, T>& point, const vec<3, T>& plane_origin, const vec<3, T>& plane_normal)
	{
		T d = -dot(plane_normal, plane_origin);
		T distance = dot(plane_normal, point) + d;
		return ProjectPointToPlaneResult3D{ .point = point - plane_normal * distance, .distance = distance };
	}

	template<typename T>
	inline vec<3, T> Math<T>::project_point_to_line(const vec<3, T>& point, const vec<3, T>& line_p1, const vec<3, T>& line_p2)
	{
		// TODO: this function should clamp point to line
		vec<3, T> ab = line_p2 - line_p1;
		real t = dot(point - line_p1, ab) / dot(ab, ab);
		t = CLAMP(t, 0.0f, 1.0f);
		return line_p1 + ab * t;
	}

	//template<typename T>
	//inline T Math<T>::distance_to_line(const vec<2, T>& point, const vec<2, T>& line_a, const vec<2, T>& line_b)
	//{
	//	vec<2, T> ab = line_b - line_a;
	//	T t = dot(point - line_a, ab) / dot(ab, ab);
	//	t = MIN(MAX(t, 0.0f), 1.0f);
	//	vec<2, T> projected_point = line_a + ab * t;
	//	T dist = length(point - projected_point);
	//	return dist;
	//}

	template<typename T>
	inline T Math<T>::distance_to_plane(const vec<2, T>& point, const vec<2, T>& plane_origin, const vec<2, T>& plane_normal)
	{
		T dist = math::dot(plane_normal, point - plane_origin);
		return dist;
	}

	template<typename T>
	inline T Math<T>::distance_to_plane(const vec<3, T>& point, const vec<3, T>& plane_origin, const vec<3, T>& plane_normal)
	{
		T dist = math::dot(plane_normal, point - plane_origin);
		return dist;
	}

	template<typename T>
	inline vec<2, T> Math<T>::perp(vec<2, T> const& vec)
	{
		return { -vec.y, vec.x };
	}

	template<typename T>
	inline vec<3, T> Math<T>::perp(vec<3, T> const& vector)
	{
		if (::abs(dot(normalize(vector), (vec<3, T>)RIGHT)) > 0.999f)
		{
			return normalize(cross(vector, UP)) * length(vector);
		}

		return normalize(cross(vector, (vec<3, T>)RIGHT)) * length(vector);
	}

	template<typename T>
	inline mat<4, 4, T> Math<T>::rotate_about_vector(vec<3, T> const& from, vec<3, T> const& to)
	{
		constexpr T epsilon = 0.000001f;
		mat<4, 4, T> m;
		T d = dot(from, to);

		if (::abs(d) > 1.0f - epsilon) // TODO: case -1.0f + epsilon is broken
		{
			m = mat<4, 4, T>(1);
		}
		else
		{
			vec<3, T> forward_to_circle = normalize(cross(from, to));
			T angle_to_circle = acos(d);
			m = rotate(angle_to_circle, forward_to_circle);
		}

		return m;
	}

	template<typename T>
	inline T Math<T>::sign(vec<2, T> const& p1, vec<2, T> const& p2, vec<2, T> const& p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}

	template<typename T>
	inline b8 Math<T>::is_point_inside_triangle(vec<2, T> const& pt, vec<2, T> const& v1, vec<2, T> const& v2, vec<2, T> const& v3)
	{
		T d1, d2, d3;
		b8 has_neg, has_pos;

		d1 = sign(pt, v1, v2);
		d2 = sign(pt, v2, v3);
		d3 = sign(pt, v3, v1);

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	//template<typename T>
	//inline T Math<T>::arc_length(vector<vec<2, T>> const& arc, b8 closed)
	//{
	//	T result = 0.0f;
	//
	//	for (s32 i = 0; i < arc.size() - 1; i++)
	//	{
	//		auto& p1 = arc[i];
	//		auto& p2 = arc[i + 1];
	//
	//		result += length(p1 - p2);
	//	}
	//
	//	if (closed)
	//	{
	//		result += length(arc.front() - arc.back());
	//	}
	//
	//	return result;
	//}

	template<typename T>
	template<typename _It>
	inline T Math<T>::arc_length(_It begin, _It end, b8 closed)
	{
		if (begin == end || std::next(begin) == end)
		{
			return 0;
		}

		T result = 0;

		_It i = begin;
		for (_It j = std::next(begin); j != end; j++)
		{
			if constexpr (requires{ result += length((vec<2, T> const&)(*j) - ((vec<2, T> const&)(*i))); })
			{
				result += length((vec<2, T> const&)(*j) - ((vec<2, T> const&)(*i)));
			}
			else if constexpr (requires{ result += length((vec<3, T> const&)(*j) - ((vec<3, T> const&)(*i))); })
			{
				result += length((vec<3, T> const&)(*j) - ((vec<3, T> const&)(*i)));
			}
			else
			{
				static_assert(false);
			}

			i = j;
		}

		if (closed)
		{
			if constexpr (requires{ result += length((vec<2, T> const&)(*begin) - ((vec<2, T> const&)(*std::prev(end)))); })
			{
				result += length((vec<2, T> const&)(*begin) - ((vec<2, T> const&)(*std::prev(end))));
			}
			else if constexpr (requires{ result += length((vec<3, T> const&)(*begin) - ((vec<3, T> const&)(*std::prev(end)))); })
			{
				result += length((vec<3, T> const&)(*begin) - ((vec<3, T> const&)(*std::prev(end))));
			}
			else
			{
				static_assert(false);
			}
		}

		return result;
	}

	template<typename T>
	inline T Math<T>::triangle_area(vec<2, T> const& p1, vec<2, T> const& p2, vec<2, T> const& p3)
	{
		T a = length(p1 - p2);
		T b = length(p2 - p3);
		T c = length(p3 - p1);
		T s = (a + b + c);
		T area = std::sqrt(s * (s - a) * (s - b) * (s - c));
		return area;
	}

	template<typename T>
	inline LineOfIntersecionBetween2PlanesResult<T> Math<T>::line_of_intersection_between_to_planes(vec<3, T> const& origin_a, vec<3, T> const& normal_a, vec<3, T> const& origin_b, vec<3, T> const& normal_b)
	{
		const vec<3, T> p3_normal = cross(normal_a, normal_b);
		const T det = length(p3_normal) * length(p3_normal);

		if (::abs(det) > NIGHT_EPSILON_MEDIUM)
		{
			T p1_d = distance_to_plane(vec<3, T>(0), origin_a, normal_a);
			T p2_d = distance_to_plane(vec<3, T>(0), origin_b, normal_b);
			vec<3, T> r_point = ((cross(p3_normal, normal_b) * p1_d) + (cross(normal_a, p3_normal) * p2_d)) / det;
			vec<3, T> r_normal = p3_normal;
			return { r_point, r_normal };
		}
		else
		{
			return { vec<3, T>(0), vec<3, T>(0) };
		}
	}

	template<typename T>
	inline EOrientation Math<T>::orientation(const vec<2, T>& a, const vec<2, T>& b, const vec<2, T>& c)
	{
		T o = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);

		if (o < 0.0f)
		{
			return EOrientation::CounterClockwise;
		}
		else if (o > 0.0f)
		{
			return EOrientation::Clockwise;
		}
		else
		{
			return EOrientation::Colinear;
		}
	}

	//template<typename T>
	//inline T Math<T>::ease_in(T t, T exponent)
	//{
	//	return pow(t, exponent);
	//}
	//
	//template<typename T>
	//inline T Math<T>::ease_out(T t, T exponent)
	//{
	//	return 1.0f - pow(1.0f - t, exponent / 1.0f);
	//}

	template<typename T>
	inline T Math<T>::ease(T t, T exponent)
	{
		if (t < 0.5f)
		{
			t = pow(t * 2, exponent) / 2;
		}
		else if (t > 0.5f)
		{
			t = 1.0f - pow(1.0f - (t - 0.5f) * 2, exponent) / 2;
		}

		return t;
	}

#if 0
	real normalize_to_range(real value, real min, real max)
	{
		ASSERT(max - min != 0.0f);
		return (value - min) / (max - min);
	}

	real lerp(real a, real b, real t)
	{
		return a + (b - a) * t;
	}

	vec2 lerp(vec2 a, vec2 b, real t)
	{
		return a + (b - a) * t;
	}

	vec3 lerp(vec3 a, vec3 b, real t)
	{
		return a + (b - a) * t;
	}

	real ilerp(real a, real b, real c)
	{
		return (c - a) / (b - a);
	}

	real ilerp(vec2 a, vec2 b, vec2 c)
	{
		return math::length(c - a) / math::length(b - a);
	}

	real ilerp(vec3 a, vec3 b, vec3 c)
	{
		return math::length(c - a) / math::length(b - a);
	}

	real dampen(real a, real b, real t, real delta)
	{
		t = 1.0f - pow(t, delta);
		return lerp(a, b, t);
	}

	real smoothstep(real a, real b, real t)
	{
		real v1 = t * t;
		real v2 = 1.0f - (1.0f - t) * (1.0f - t);
		return lerp(a, b, lerp(v1, v2, t));
	}

	vec2 bezier(const vec2& a, const vec2& b, const vec2& c, real t)
	{
		const real x = (1.f - t) * (1.f - t) * a.x + 2.f * (1.f - t) * t * b.x + t * t * c.x;
		const real y = (1.f - t) * (1.f - t) * a.y + 2.f * (1.f - t) * t * b.y + t * t * c.y;

		return { x, y };
	}

	vec2 slerp(const vec2& a, const vec2& b, real t)
	{
		real d = math::dot(a, b);
		d = CLAMP(d, -1.0f, 1.0f);
		real theta = acos(d) * t;
		vec2 relative = math::normalize(b - a * d);
		return (a * cos(theta)) + (relative * sin(theta));
	}

	real angle(const vec2 v)
	{
		return atan2(v.x, v.y);
	}

	real angle_clockwise(const vec2& a, const vec2& b)
	{
		real d = a.x * b.x + a.y * b.y;
		real det = a.x * b.y - a.y * b.x;
		real angle = atan2(-det, -d) + R_PI;
		return angle;
	}
	real angle_counter_clockwise(const vec2& a, const vec2& b)
	{
		real d = a.x * b.x + -a.y * -b.y;
		real det = a.x * -b.y - -a.y * b.x;
		real angle = atan2(-det, -d) + R_PI;
		return angle;
	}

	real perp_dot(const vec2& a, const vec2& b)
	{
		return math::dot({-b.y, b.x}, a);
	}

	//vec2 math::cross(const vec2& a, const vec2& b)
	//{
	//	vec3 aa = { a.x, a.y, 0.0f };
	//	vec3 bb = { b.x, b.y, 0.0f };
	//	vec3 c1 = math::cross(aa, bb);

	//	return vec2{ c1.x, c1.y };
	//}

	//vec2 triple_cross(const vec2& a, const vec2& b, const vec2& c)
	//{
	//	vec3 aa = { a.x, a.y, 0.0f };
	//	vec3 bb = { b.x, b.y, 0.0f };
	//	vec3 cc = { c.x, c.y, 0.0f };
	//	vec3 c1 = math::cross(aa, bb);
	//	vec3 c2 = math::cross(c1, cc);
	//
	//	return vec2{ c2.x, c2.y };
	//}
	//
	//vec3 triple_cross(const vec3& a, const vec3& b, const vec3& c)
	//{
	//	vec3 c1 = math::cross(a, b);
	//	vec3 c2 = math::cross(c1, c);
	//
	//	return c2;
	//}

	// TODO: make sure this function works
	ProjectPointToPlaneResult2D project_point_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal)
	{
		real d = -math::dot(plane_normal, plane_origin);
		real distance = math::dot(plane_normal, point) + d;
		return { .point = point - plane_normal * distance, .distance = distance };
	}

	vec2 project_point_to_line(const vec2& point, const vec2& line_p1, const vec2& line_p2)
	{
		vec2 ab = line_p2 - line_p1;
		real t = math::dot(point - line_p1, ab) / math::dot(ab, ab);
		t = CLAMP(t, 0.0f, 1.0f);
		return line_p1 + ab * t;
	}

	ProjectPointToPlaneResult3D project_point_to_plane(const vec3& point, const vec3& plane_origin, const vec3& plane_normal)
	{
		real d = -math::dot(plane_normal, plane_origin);
		real distance = math::dot(plane_normal, point) + d;
		return { .point = point - plane_normal * distance, .distance = distance };
	}

	vec3 project_point_to_line(const vec3& point, const vec3& line_a, const vec3& line_b)
	{
		vec3 ab = line_b - line_a;
		vec3 ap = point - line_a;
		return line_a + math::dot(ap, ab) / math::dot(ab, ab) * ab;
	}

	// TODO: make signed
	real distance_to_line(const vec2& point, const vec2& line_a, const vec2& line_b)
	{
		vec2 ab = line_b - line_a;
		real t = math::dot(point - line_a, ab) / math::dot(ab, ab);
		t = MIN(MAX(t, 0.0f), 1.0f);
		vec2 projected_point = line_a + ab * t;
		real dist = math::length(point - projected_point);
		return dist;
	}

	// TODO: make signed
	real distance_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal)
	{
		real dist = math::dot(plane_normal, point - plane_origin);
		return dist;
	}

	real distance_to_plane(const vec3& point, const vec3& plane_origin, const vec3& plane_normal)
	{
		real dist = math::dot(plane_normal, point - plane_origin);
		return dist;
	}

	vec2 NIGHT_API perpendicular_vector(vec2 const& vec)
	{
		return { -vec.y, vec.x };
	}

	// TODO: find better way
	vec3  perpendicular_vector(vec3 const& vec)
	{
		if (abs(math::dot(math::normalize(vec), RIGHT)) > 0.999f)
		{
			return math::normalize(math::cross(vec, UP)) * math::length(vec);
		}

		return math::normalize(math::cross(vec, RIGHT)) * math::length(vec);
	}

	dvec3 NIGHT_API perpendicular_vector(dvec3 const& vec)
	{
		if (abs(dmath::dot(dmath::normalize(vec), (dvec3)RIGHT)) > 0.999f)
		{
			return dmath::normalize(dmath::cross(vec, (dvec3)UP)) * dmath::length(vec);
		}

		return dmath::normalize(dmath::cross(vec, (dvec3)RIGHT)) * dmath::length(vec);
	}

	mat4 rotate_about_vector(vec3 const& from, vec3 const& to)
	{
		constexpr real epsilon = 0.000001f;
		mat4 m;
		real d = math::dot(from, to);

		if (abs(d) > 1.0f - epsilon) // TODO: case -1.0f + epsilon is broken
		{
			m = mat4(1);
		}
		else
		{
			vec3 forward_to_circle = math::normalize(math::cross(from, to));
			real angle_to_circle = acos(d);
			m = math::rotate(angle_to_circle, forward_to_circle);
		}

		return m;
	}

	b8  is_point_inside_triangle(vec2 const& pt, vec2 const& v1, vec2 const& v2, vec2 const& v3)
	{
		real d1, d2, d3;
		b8 has_neg, has_pos;

		d1 = sign(pt, v1, v2);
		d2 = sign(pt, v2, v3);
		d3 = sign(pt, v3, v1);

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	real arc_length(vector<vec2> const& arc, b8 closed)
	{
		real result = 0.0f;

		for (s32 i = 0; i < arc.size() - 1; i++)
		{
			auto& p1 = arc[i];
			auto& p2 = arc[i + 1];

			result += math::length(p1 - p2);
		}

		if (closed)
		{
			result += math::length(arc.front() - arc.back());
		}

		return result;
	}

	real NIGHT_API triangle_area(vec2 const& p1, vec2 const& p2, vec2 const& p3)
	{
		real a = math::length(p1 - p2);
		real b = math::length(p2 - p3);
		real c = math::length(p3 - p1);
		real s = (a + b + c);
		real area = std::sqrt(s * (s - a) * (s - b) * (s - c));
		return area;
	}

	LineOfIntersecionBetween2PlanesResult line_of_intersection_between_to_planes(vec3 const& origin_a, vec3 const& normal_a, vec3 const& origin_b, vec3 const& normal_b)
	{
		const vec3 p3_normal = math::cross(normal_a, normal_b);
		const float det = math::length(p3_normal) * math::length(p3_normal);

		if (abs(det) > NIGHT_EPSILON_MEDIUM)
		{
			real p1_d = distance_to_plane(vec3(0), origin_a, normal_a);
			real p2_d = distance_to_plane(vec3(0), origin_b, normal_b);
			vec3 r_point = ((math::cross(p3_normal, normal_b) * p1_d) + (math::cross(normal_a, p3_normal) * p2_d)) / det;
			vec3 r_normal = p3_normal;
			return { r_point, r_normal };
		}
		else
		{
			return { vec3(0), vec3(0) };
		}
	}

	EOrientation orientation(const vec2& a, const vec2& b, const vec2& c)
	{
		real o = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);

		if (o < 0.0f)
		{
			return EOrientation::CounterClockwise;
		}
		else if (o > 0.0f)
		{
			return EOrientation::Clockwise;
		}
		else
		{
			return EOrientation::Colinear;
		}
	}

	DecomposedTransform decompose(mat4 const& x)
	{
		//DecomposedTransform result;

		//math::decompose(x, result.scale, result.rotation, result.translation, result.skew, result.perspective);

		//return result;
		return math::decompose(x);
	}

	mat4 compose(DecomposedTransform const& decomp)
	{
		//mat4 result = mat4(1);
		//result = math::scale(decomp.scale) * result;
		//result = math::quat_to_mat4(decomp.rotation) * result;
		//result = math::translate(decomp.translation) * result;
		//return result;
		return math::compose(decomp);
	}

	s32 NIGHT_API _mod(s32 i, s32 mod)
	{
		while (i < mod)
		{
			i += mod;
		}

		return i % mod;
	}

	real sign(vec2 const& p1, vec2 const& p2, vec2 const& p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}
#endif
}