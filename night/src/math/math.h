#pragma once

#include "core.h"
//#include "types/types.h"
//#include "stl/stl.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"

#define NIGHT_MATH_EPSILON 1e-04f


namespace night
{

	#ifdef NIGHT_USE_DOUBLE_PRECISION
	#define R_PI	3.14159265358979323846
	#else
	#define R_PI	3.14159265358979323846f
	#endif

	#define RADIANS(x) glm::radians((float)x)
	#define DEGREES(x) glm::degrees((float)x)


#define NORMALIZE(x, lower_bound, upper_bound) (abs(x - lower_bound) / (upper_bound - lower_bound)) // TODO: this does not work

#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef glm::dvec1  vec1;
	typedef glm::dvec2  vec2;
	typedef glm::dvec3  vec3;
	typedef glm::dvec4  vec4;
#else
	typedef glm::vec1  vec1;
	typedef glm::vec2  vec2;
	typedef glm::vec3  vec3;
	typedef glm::vec4  vec4;
#endif

#define FORWARD vec3(0, 0, 1)
#define BACKWARDS vec3(0, 0, -1)
#define LEFT vec3( -1, 0, 0 )
#define RIGHT vec3( 1, 0, 0 )
#define UP vec3(0, 1, 0)
#define DOWN vec3(0, -1, 0)
#define ORIGIN vec3(0, 0, 0)

	typedef glm::ivec1  ivec1;
	typedef glm::ivec2  ivec2;
	typedef glm::ivec3  ivec3;
	typedef glm::ivec4  ivec4;

	typedef glm::uvec1  uvec1;
	typedef glm::uvec2  uvec2;
	typedef glm::uvec3  uvec3;
	typedef glm::uvec4  uvec4;

	typedef glm::fvec1  fvec1;
	typedef glm::fvec2  fvec2;
	typedef glm::fvec3  fvec3;
	typedef glm::fvec4  fvec4;

	typedef glm::dvec1  dvec1;
	typedef glm::dvec2  dvec2;
	typedef glm::dvec3  dvec3;
	typedef glm::dvec4  dvec4;

#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef glm::dmat2		 mat2;
	typedef glm::dmat2x3	 mat2x3;
	typedef glm::dmat2x4	 mat2x4;
	typedef glm::dmat3x2	 mat3x2;
	typedef glm::dmat3		 mat3;
	typedef glm::dmat3x4	 mat3x4;
	typedef glm::dmat4x2	 mat4x2;
	typedef glm::dmat4x3	 mat4x3;
	typedef glm::dmat4		 mat4;
#else
	typedef glm::mat2		 mat2;
	typedef glm::mat2x3		 mat2x3;
	typedef glm::mat2x4		 mat2x4;
	typedef glm::mat3x2		 mat3x2;
	typedef glm::mat3		 mat3;
	typedef glm::mat3x4		 mat3x4;
	typedef glm::mat4x2		 mat4x2;
	typedef glm::mat4x3		 mat4x3;
	typedef glm::mat4		 mat4;
#endif

	typedef glm::fmat2		 fmat2;
	typedef glm::fmat2x3	 fmat2x3;
	typedef glm::fmat2x4	 fmat2x4;
	typedef glm::fmat3x2	 fmat3x2;
	typedef glm::fmat3		 fmat3;
	typedef glm::fmat3x4	 fmat3x4;
	typedef glm::fmat4x2	 fmat4x2;
	typedef glm::fmat4x3	 fmat4x3;
	typedef glm::fmat4		 fmat4;

	typedef glm::dmat2		 dmat2;
	typedef glm::dmat2x3	 dmat2x3;
	typedef glm::dmat2x4	 dmat2x4;
	typedef glm::dmat3x2	 dmat3x2;
	typedef glm::dmat3		 dmat3;
	typedef glm::dmat3x4	 dmat3x4;
	typedef glm::dmat4x2	 dmat4x2;
	typedef glm::dmat4x3	 dmat4x3;
	typedef glm::dmat4		 dmat4;

#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef glm::dquat quat;
#else
	typedef glm::fquat quat;
#endif

	// this function does not work
	real NIGHT_API normalize_to_range(real value, real min, real max);

	real NIGHT_API lerp(real a, real b, real t);

	real NIGHT_API dampen(real a, real b, real t, real delta);

	real NIGHT_API smoothstep(real a, real b, real t);

	vec2 NIGHT_API bezier(const vec2& a, const vec2& b, const vec2& c, real t);

	vec2 NIGHT_API slerp(const vec2& a, const vec2& b, real t);

	real NIGHT_API angle(const vec2 v);

	real NIGHT_API angle_clockwise(const vec2& a, const vec2& b);

	real NIGHT_API angle_counter_clockwise(const vec2& a, const vec2& b);

	// TODO: remove
	real NIGHT_API perp_dot(const vec2& a, const vec2& b);

	vec2 NIGHT_API cross(const vec2& a, const vec2& b);

	vec2 NIGHT_API triple_cross(const vec2& a, const vec2& b, const vec2& c);

	vec3 NIGHT_API triple_cross(const vec3& a, const vec3& b, const vec3& c);

	struct ProjectPointToPlaneResult2D // TODO: clean this up
	{
		vec2 point{};
		real t{};
	};

	ProjectPointToPlaneResult2D NIGHT_API project_point_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal);

	struct ProjectPointToPlaneResult3D // TODO: clean this up
	{
		vec3 point{};
		real distance{};
	};

	ProjectPointToPlaneResult3D NIGHT_API project_point_to_plane(const vec3& point, const vec3& plane_origin, const vec3& plane_normal);
	vec3 NIGHT_API project_point_to_line(const vec3& point, const vec3& line_a, const vec3& line_b);

	real NIGHT_API distance_to_line(const vec2& point, const vec2& line_a, const vec2& line_b);
	real NIGHT_API distance_to_plane(const vec2& point, const vec2& plane_origin, const vec2& plane_normal);

	real NIGHT_API distance_to_plane(const vec3& point, const vec3& plane_origin, const vec3& plane_normal);

	//struct LineCircleIntersectionResult
	//{
	//	vec2 contacts[2]{};
	//	real t[2]{ -2.0f };
	//	s32 count{ 0 };
	//};

	vec3 NIGHT_API perpendicular_vector(vec3 const& vec);

	// this function assumed the vectors are normalized
	mat4 NIGHT_API rotate_about_vector(vec3 const& from, vec3 const& to);

	real NIGHT_API sign(vec2 const& p1, vec2 const& p2, vec2 const& p3);

	u8 NIGHT_API is_point_inside_triangle(vec2 const& pt, vec2 const& v1, vec2 const& v2, vec2 const& v3);

	real NIGHT_API arc_length(vector<vec2> const& arc, u8 closed);

	struct LineOfIntersecionBetween2PlanesResult
	{
		vec3 origin;
		vec3 direction;
	};

	LineOfIntersecionBetween2PlanesResult NIGHT_API line_of_intersection_between_to_planes(vec3 const& origin_a, vec3 const& normal_a, vec3 const& origin_b, vec3 const& normal_b);
	
	enum class EOrientation
	{
		Colinear = 0,
		Clockwise = 1,
		CounterClockwise = 2
	};

	EOrientation NIGHT_API orientation(const vec2& a, const vec2& b, const vec2& c);

	struct DecomposedTransform
	{
		vec3 scale{ 1, 1, 1 };
		vec3 translation{ 0, 0, 0 };
		quat rotation{ quat(vec3(0, 0, 0)) };
		vec3 skew{};
		vec4 perspective{};
	};

	DecomposedTransform NIGHT_API decompose(mat4 const& x);

	mat4 compose(DecomposedTransform const& decomp);
}