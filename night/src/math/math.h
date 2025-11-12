#pragma once

#include "core.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/glm.hpp"
#define NIGHT_MATH_EPSILON 1e-04f

#include "log/log.h"

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

	template<s32 L, typename T>
	using vec = glm::vec<L, T>;

	using vec2 = vec<2, real>;
	using vec3 = vec<3, real>;
	using vec4 = vec<4, real>;

	using dvec2 = vec<2, r64>;
	using dvec3 = vec<3, r64>;
	using dvec4 = vec<4, r64>;

	using fvec2 = vec<2, r32>;
	using fvec3 = vec<3, r32>;
	using fvec4 = vec<4, r32>;

	using ivec2 = vec<2, s32>;
	using ivec3 = vec<3, s32>;
	using ivec4 = vec<4, s32>;

	using uvec2 = vec<2, u32>;
	using uvec3 = vec<3, u32>;
	using uvec4 = vec<4, u32>;

	using bvec2 = vec<2, u8>;
	using bvec3 = vec<3, u8>;
	using bvec4 = vec<4, u8>;

	template<s32 L1, s32 L2, typename T>
	using mat = glm::mat<L1, L2, T>;

	using mat2 = mat<2, 2, real>;
	using mat2x3 = mat<2, 3, real>;
	using mat2x4 = mat<2, 4, real>;
	using mat3x2 = mat<3, 2, real>;
	using mat3 = mat<3, 3, real>;
	using mat3x4 = mat<3, 4, real>;
	using mat4x2 = mat<4, 2, real>;
	using mat4x3 = mat<4, 3, real>;
	using mat4 = mat<4, 4, real>;

	using dmat2 = mat<2, 2, r64>;
	using dmat2x3 = mat<2, 3, r64>;
	using dmat2x4 = mat<2, 4, r64>;
	using dmat3x2 = mat<3, 2, r64>;
	using dmat3 = mat<3, 3, r64>;
	using dmat3x4 = mat<3, 4, r64>;
	using dmat4x2 = mat<4, 2, r64>;
	using dmat4x3 = mat<4, 3, r64>;
	using dmat4 = mat<4, 4, r64>;

	using fmat2 = mat<2, 2, r32>;
	using fmat2x3 = mat<2, 3, r32>;
	using fmat2x4 = mat<2, 4, r32>;
	using fmat3x2 = mat<3, 2, r32>;
	using fmat3 = mat<3, 3, r32>;
	using fmat3x4 = mat<3, 4, r32>;
	using fmat4x2 = mat<4, 2, r32>;
	using fmat4x3 = mat<4, 3, r32>;
	using fmat4 = mat<4, 4, r32>;

	template<typename T>
	using qua = glm::qua<T>;

	using quat = qua<real>;
	using dquat = qua<r64>;
	using fquat = qua<r32>;

#if 0
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
#endif

#define FORWARD vec3(0, 0, 1)
#define BACKWARDS vec3(0, 0, -1)
#define LEFT vec3( -1, 0, 0 )
#define RIGHT vec3( 1, 0, 0 )
#define UP vec3(0, 1, 0)
#define DOWN vec3(0, -1, 0)
#define ORIGIN vec3(0, 0, 0)

	template<typename T = real>
	struct ProjectPointToPlaneResult2D
	{
		vec<2, T> point{};
		T distance{};
	};

	template<typename T = real>
	struct ProjectPointToPlaneResult3D
	{
		vec<3, T> point{};
		T distance{};
	};

	template<typename T = real>
	struct LineOfIntersecionBetween2PlanesResult
	{
		vec<3, T> origin;
		vec<3, T> direction;
	};

	template<typename T = real>
	struct DecomposedTransform
	{
		vec<3, T> scale{ 1, 1, 1 };
		vec<3, T> translation{ 0, 0, 0 };
		qua<T> rotation{ qua<T>(vec<3, T>(0, 0, 0)) };
		vec<3, T> skew{};
		vec<4, T> perspective{};
	};

	enum struct EOrientation : s32
	{
		CounterClockwise = -1,
		Colinear = 0,
		Clockwise = 1
	};

	template<typename T>
	struct Math
	{

#pragma region GLM_WRAPPER_FUNCTIONS
		inline static vec<2, T> normalize(vec<2, T> const& vec) { return glm::normalize(vec); }
		inline static vec<3, T> normalize(vec<3, T> const& vec) { return glm::normalize(vec); }
		inline static vec<4, T> normalize(vec<4, T> const& vec) { return glm::normalize(vec); }

		inline static T dot(vec<2, T> const& a, vec<2, T> const& b) { return glm::dot(a, b); }
		inline static T dot(vec<3, T> const& a, vec<3, T> const& b) { return glm::dot(a, b); }
		inline static T dot(vec<4, T> const& a, vec<4, T> const& b) { return glm::dot(a, b); }

		inline static vec<3, T> cross(vec<3, T> const& a, vec<3, T> const& b) { return glm::cross(a, b); }

		inline static vec<2, T> triple_cross(vec<2, T> const& a, vec<2, T> const& b, vec<2, T> const& c)
		{
			vec<3, T> aa = { a.x, a.y, 0.0f };
			vec<3, T> bb = { b.x, b.y, 0.0f };
			vec<3, T> cc = { c.x, c.y, 0.0f };
			vec<3, T> c1 = Math<T>::cross(aa, bb);
			vec<3, T> c2 = Math<T>::cross(c1, cc);

			return vec<2, T>{ c2.x, c2.y };
		}

		inline static vec<3, T> triple_cross(vec<3, T> const& a, vec<3, T> const& b, vec<3, T> const& c)
		{
			vec<3, T> c1 = Math<T>::cross(a, b);
			vec<3, T> c2 = Math<T>::cross(c1, c);

			return c2;
		}

		inline static T length(vec<2, T> const& vec) { return glm::length(vec); }
		inline static T length(vec<3, T> const& vec) { return glm::length(vec); }
		inline static T length(vec<4, T> const& vec) { return glm::length(vec); }

		inline static mat<4, 4, T> translate(vec<3, T> const& translation)
		{
			return glm::translate(translation);
		}

		inline static mat<4, 4, T> scale(vec<3, T> const& scale)
		{
			return glm::scale(scale);
		}

		inline static mat<4, 4, T> rotate(T angle, vec<3, T> const& axis)
		{
			return glm::rotate(angle, axis);
		}

		inline static vec<2, T> rotate(vec<2, T> const& vec, T angle)
		{
			return glm::rotate(vec, angle);
		}

		inline static mat<4, 4, T> quat_to_mat4(qua<T> const& quat)
		{
			return glm::mat4_cast(quat);
		}

		inline static DecomposedTransform<T> decompose(mat<4, 4, T> const& matrix)
		{
			DecomposedTransform<T> result;
			glm::decompose(matrix, result.scale, result.rotation, result.translation, result.skew, result.perspective);
			return result;
		}

		inline static mat<4, 4, T> compose(DecomposedTransform<T> const& decomp)
		{
			mat<4, 4, T> result = mat<4, 4, T>(1);
			result = Math<T>::scale(decomp.scale) * result;
			result = Math<T>::quat_to_mat4(decomp.rotation) * result;
			result = Math<T>::translate(decomp.translation) * result;
			return result;
		}

		inline static qua<T> angle_axis(T radians, vec<3, T> const& axis)
		{
			return glm::angleAxis(radians, axis);
		}

		inline static mat<4, 4, T> look_at(vec<3, T> const& translation, vec<3, T> const& look_at, vec<3, T> const& up)
		{
			return glm::lookAt(translation, look_at, up);
		}

		inline static mat<4, 4, T> perspective(T fov, T aspect, T near_clip, T far_clip)
		{
			return glm::perspective(fov, aspect, near_clip, far_clip);
		}

		inline static mat<4, 4, T> ortho(T left, T right, T bottom, T top, T near_clip, T far_clip)
		{
			return glm::ortho(left, right, bottom, top, near_clip, far_clip);
		}

		inline static vec<3, T> project(vec<3, T> const& point, mat<4, 4, T> const& model, mat<4, 4, T> const& proj, vec<4, T> const& view_port)
		{
			return glm::project(point, model, proj, view_port);
		}

		inline static vec<3, T> unproject(vec<3, T> const& point, mat<4, 4, T> const& model, mat<4, 4, T> const& proj, vec<4, T> const& view_port)
		{
			return glm::unProject(point, model, proj, view_port);
		}

		inline static mat<4, 4, T> inverse(mat<4, 4, T> const& matrix) { return glm::inverse(matrix); }
		
		inline static qua<T> inverse(qua<T> const& quat) { return glm::inverse(quat); };

		inline static vec<3, T> euler_angles(qua<T> quat)
		{
			return glm::eulerAngles(quat);
		}

#pragma endregion

		inline static T lerp(T a, T b, T t);
		inline static vec<2, T> lerp(vec<2, T> a, vec<2, T> b, T t);
		inline static vec<3, T> lerp(vec<3, T> a, vec<3, T> b, T t);

		inline static T ilerp(T a, T b, T c);
		inline static T ilerp(vec<2, T> a, vec<2, T> b, vec<2, T> c);
		inline static T ilerp(vec<3, T> a, vec<3, T> b, vec<3, T> c);

		inline static T dampen(T a, T b, T t, T delta);
		inline static T smoothstep(T a, T b, T t);
		inline static vec<2, T> bezier(const vec<2, T>& a, const vec<2, T>& b, const vec<2, T>& c, T t);

		//inline static vec<2, T> slerp(const vec<2, T>& a, const vec<2, T>& b, T t);

		// TODO: cw and ccw may be reverse
		inline static T angle_clockwise(const vec<2, T>& a, const vec<2, T>& b);
		inline static T angle_counter_clockwise(const vec<2, T>& a, const vec<2, T>& b);

		inline static ProjectPointToPlaneResult2D<T> project_point_to_plane(const vec<2, T>& point, const vec<2, T>& plane_origin, const vec<2, T>& plane_normal);
		inline static ProjectPointToPlaneResult3D<T> project_point_to_plane(const vec<3, T>& point, const vec<3, T>& plane_origin, const vec<3, T>& plane_normal);

		inline static vec<2, T> project_point_to_line(const vec<2, T>& point, const vec<2, T>& line_p1, const vec<2, T>& line_p2);
		inline static vec<3, T> project_point_to_line(const vec<3, T>& point, const vec<3, T>& line_a, const vec<3, T>& line_b);

		//inline static T distance_to_line(const vec<2, T>& point, const vec<2, T>& line_a, const vec<2, T>& line_b);
		
		inline static T distance_to_plane(const vec<2, T>& point, const vec<2, T>& plane_origin, const vec<2, T>& plane_normal);
		inline static T distance_to_plane(const vec<3, T>& point, const vec<3, T>& plane_origin, const vec<3, T>& plane_normal);

		inline static vec<2, T> perp(vec<2, T> const& vec);
		inline static vec<3, T> perp(vec<3, T> const& vec);

		// this function assumed the vectors are normalized
		// TODO: this function has broken edge case
		inline static mat<4, 4, T> rotate_about_vector(vec<3, T> const& from, vec<3, T> const& to);

		inline static T sign(vec<2, T> const& p1, vec<2, T> const& p2, vec<2, T> const& p3);

		inline static u8 is_point_inside_triangle(vec<2, T> const& pt, vec<2, T> const& v1, vec<2, T> const& v2, vec<2, T> const& v3);

		template<typename _It>
		inline static T arc_length(_It begin, _It end, u8 closed = false);

		inline static T triangle_area(vec<2, T> const& p1, vec<2, T> const& p2, vec<2, T> const& p3);

		// TODO: move to intersection.h
		inline static LineOfIntersecionBetween2PlanesResult<T> line_of_intersection_between_to_planes(vec<3, T> const& origin_a, vec<3, T> const& normal_a, vec<3, T> const& origin_b, vec<3, T> const& normal_b);

		inline static EOrientation orientation(const vec<2, T>& a, const vec<2, T>& b, const vec<2, T>& c);
	};

	using math = Math<real>;
	using dmath = Math<r64>;
	using fmath = Math<r32>;

	// placeholder function for modding negative numbers:
	inline s32 _mod(s32 i, s32 mod)
	{
		while (i < mod)
		{
			i += mod;
		}

		return i % mod;
	}

}

#include "math.inl"

namespace std
{
	template<>
	struct hash<night::ivec2>
	{
		uint64_t operator()(const night::ivec2& key) const
		{
			return hash<uint64_t>()(key.x ^ key.y);
		}
	};
};