
#include "nightpch.h"
#include "math.h"
//#include "stl/stl.h"
#include "log/log.h"

namespace night
{

	real normalize_to_range(real value, real min, real max)
	{
		ASSERT(max - min != 0.0f);
		return (value - min) / (max - min);
	}

	real lerp(real a, real b, real t)
	{
		return a + (b - a) * t;
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
		real d = dot(a, b);
		d = CLAMP(d, -1.0f, 1.0f);
		real theta = acos(d) * t;
		vec2 relative = normalize(b - a * d);
		return (a * cos(theta)) + (relative * sin(theta));
	}

	real  angle(const vec2 v)
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
		return dot({-b.y, b.x}, a);
	}

	vec2 cross(const vec2& a, const vec2& b)
	{
		vec3 aa = { a.x, a.y, 0.0f };
		vec3 bb = { b.x, b.y, 0.0f };
		vec3 c1 = cross(aa, bb);

		return vec2{ c1.x, c1.y };
	}

	vec2 triple_cross(const vec2& a, const vec2& b, const vec2& c)
	{
		vec3 aa = { a.x, a.y, 0.0f };
		vec3 bb = { b.x, b.y, 0.0f };
		vec3 cc = { c.x, c.y, 0.0f };
		vec3 c1 = cross(aa, bb);
		vec3 c2 = cross(c1, cc);

		return vec2{ c2.x, c2.y };
	}

	vec3 triple_cross(const vec3& a, const vec3& b, const vec3& c)
	{
		vec3 c1 = cross(a, b);
		vec3 c2 = cross(c1, c);

		return c2;
	}

	ProjectPointToPlaneResult2D project_point_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal)
	{
		real t = dot(point - plane_origin, plane_normal) / dot(plane_normal, plane_normal);
		vec2 projected_point = plane_origin + plane_normal * t;
		return {.point = projected_point, .t = t };
	}

	ProjectPointToPlaneResult3D  project_point_to_plane(const vec3& point, const vec3& plane_origin, const vec3& plane_normal)
	{
		real d = -dot(plane_normal, plane_origin);
		real distance = dot(plane_normal, point) + d;
		return { .point = point - plane_normal * distance, .distance = distance };
	}

	vec3 project_point_to_line(const vec3& point, const vec3& line_a, const vec3& line_b)
	{
		vec3 ab = line_b - line_a;
		vec3 ap = point - line_a;
		return line_a + dot(ap, ab) / dot(ab, ab) * ab;
	}

	// TODO: make signed
	real distance_to_line(const vec2& point, const vec2& line_a, const vec2& line_b)
	{
		vec2 ab = line_b - line_a;
		real t = dot(point - line_a, ab) / dot(ab, ab);
		t = MIN(MAX(t, 0.0f), 1.0f);
		vec2 projected_point = line_a + ab * t;
		real dist = distance(point, projected_point);
		return dist;
	}

	// TODO: make signed
	real distance_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal)
	{
		real dist = dot(plane_normal, point - plane_origin);
		return dist;
	}

	real distance_to_plane(const vec3& point, const vec3& plane_origin, const vec3& plane_normal)
	{
		real dist = dot(plane_normal, point - plane_origin);
		return dist;
	}

	// TODO: find better way
	vec3  perpendicular_vector(vec3 const& vec)
	{
		if (abs(dot(normalize(vec), RIGHT)) > 0.999f)
		{
			return normalize(cross(vec, UP)) * length(vec);
		}

		return normalize(cross(vec, RIGHT)) * length(vec);
	}

	mat4 rotate_about_vector(vec3 const& from, vec3 const& to)
	{
		constexpr real epsilon = 0.000001f;
		mat4 m;
		real d = dot(from, to);

		if (abs(d) > 1.0f - epsilon)
		{
			m = mat4(1);
		}
		else
		{
			vec3 forward_to_circle = normalize(cross(from, to));
			real angle_to_circle = acos(d);
			m = rotate(mat4(1), angle_to_circle, forward_to_circle);
		}

		return m;
	}

	u8  is_point_inside_triangle(vec2 const& pt, vec2 const& v1, vec2 const& v2, vec2 const& v3)
	{
		real d1, d2, d3;
		u8 has_neg, has_pos;

		d1 = sign(pt, v1, v2);
		d2 = sign(pt, v2, v3);
		d3 = sign(pt, v3, v1);

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	real arc_length(vector<vec2> const& arc, u8 closed)
	{
		real result = 0.0f;

		for (s32 i = 0; i < arc.size() - 1; i++)
		{
			auto& p1 = arc[i];
			auto& p2 = arc[i + 1];

			result += distance(p1, p2);
		}

		if (closed)
		{
			result += distance(arc.front(), arc.back());
		}

		return result;
	}

	LineOfIntersecionBetween2PlanesResult line_of_intersection_between_to_planes(vec3 const& origin_a, vec3 const& normal_a, vec3 const& origin_b, vec3 const& normal_b)
	{
		const vec3 p3_normal = cross(normal_a, normal_b);
		const float det = length(p3_normal) * length(p3_normal);

		if (abs(det) > NIGHT_MATH_EPSILON)
		{
			real p1_d = distance_to_plane(vec3(0), origin_a, normal_a);
			real p2_d = distance_to_plane(vec3(0), origin_b, normal_b);
			vec3 r_point = ((cross(p3_normal, normal_b) * p1_d) + (cross(normal_a, p3_normal) * p2_d)) / det;
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
		DecomposedTransform result;

		glm::decompose(x, result.scale, result.rotation, result.translation, result.skew, result.perspective);

		return result;
	}

	mat4 compose(DecomposedTransform const& decomp)
	{
		mat4 result = mat4(1);
		result = glm::scale(decomp.scale) * result;
		result = glm::mat4_cast(decomp.rotation) * result;
		result = glm::translate(decomp.translation) * result;
		return result;
	}

	real sign(vec2 const& p1, vec2 const& p2, vec2 const& p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}
}