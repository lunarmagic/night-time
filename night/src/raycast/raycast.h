#pragma once

#include "math/Math.h"

namespace night
{

	template<typename T = real>
	struct Ray2D
	{
		vec<2, T> origin{ vec<2, T>(0) };
		vec<2, T> direction{ vec<2, T>(0) };
	};

	template<typename T = real>
	struct Ray3D
	{
		vec<3, T> origin{ vec<3, T>(0) };
		vec<3, T> direction{ vec<3, T>(0) };
	};

	//using ray2d = Ray2D<real>;
	//using dray2d = Ray2D<r64>;
	//using fray2d = Ray2D<r32>;
	//
	//using ray3d = Ray3D<real>;
	//using dray3d = Ray3D<r64>;
	//using fray3d = Ray3D<r32>;

	template<typename T = real>
	struct RaycastResult2D
	{
		u8 result;

		union
		{
			array<T, 2> thetas;
			struct
			{
				T t0; // t0 is by default INFINITY
				T t1; // t1 is by default -INFINITY
			};
			T t;
		};

		union
		{
			array<vec<2, T>, 2> normals;
			struct
			{
				vec<2, T> n0;
				vec<2, T> n1;
			};
			vec<2, T> normal;
		};

		template<s32 Which_Theta = 0>
		vec<2, T> contact(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction) const
		{
			if constexpr (Which_Theta == 0)
			{
				return ray_origin + ray_direction * t0;
			}
			else
			{
				return ray_origin + ray_direction * t1;
			}
		}
	};

	template<typename T = real>
	struct RaycastResult3D
	{
		u8 result;

		union
		{
			array<T, 2> thetas;
			struct
			{
				T t0; // t0 is by default INFINITY
				T t1; // t1 is by default -INFINITY
			};
			T t;
		};

		union
		{
			array<vec<3, T>, 2> normals;
			struct
			{
				vec<3, T> n0;
				vec<3, T> n1;
			};
			vec<3, T> normal;
		};

		template<s32 Which_Theta = 0>
		vec<3, T> contact(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction) const
		{
			if constexpr (Which_Theta == 0)
			{
				return ray_origin + ray_direction * t0;
			}
			else
			{
				return ray_origin + ray_direction * t1;
			}
		}
	};

	template<typename T = real>
	struct RaycastTriangleResult
	{
		u8 result()
		{
			return (coordinate.x >= 0.0f && coordinate.y >= 0.0f && coordinate.z >= 0.0f);
		}

		T t{ INFINITY };
		vec<3, T> coordinate{ vec<3, T>(-INFINITY) };
		vec<3, T> normal{ vec<3, T>(0) };

		vec<3, T> contact(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction) const
		{
			return ray_origin + ray_direction * t;
		}
	};

	template<typename T = real>
	struct Raycast2D
	{
		static RaycastResult2D<T> plane(vec<2, T> const& ray_origin, vec<2, T> const& ray_dir, vec<2, T> const& plane_origin, vec<2, T> const& plane_normal, T epsilon = 0.0f);
		static RaycastResult2D<T> line(vec<2, T> const& ray_origin, vec<2, T> const& ray_dir, vec<2, T> const& p1, vec<2, T> const& p2, T epsilon = 0.0f);
		static RaycastResult2D<T> circle(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, vec<2, T> const& circle_origin, T circle_radius);

		static T iraycast(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, vec<2, T> const& point);

		static vec<2, T> project_point_to_ray(const vec<2, T>& point, const vec<2, T>& ray_origin, const vec<2, T>& ray_direction);
	};

	template<typename T = real>
	struct Raycast3D
	{
		static RaycastResult3D<T> plane(vec<3, T> const& ray_origin, vec<3, T> const& ray_dir, vec<3, T> const& plane_origin, vec<3, T> const& plane_normal, T epsilon = 0.0f); // TODO: better implement this.
		static RaycastResult3D<T> sphere(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction, vec<3, T> const& sphere_origin, T sphere_radius);

		static RaycastTriangleResult<T> triangle(vec<3, T> const& ray_origin, vec<3, T> const& ray_dir, vec<3, T> const& p1, vec<3, T> const& p2, vec<3, T> const& p3, T epsilon = 0.0f);
		static RaycastResult3D<T> cylinder(vec<3, T> const& ray_origin, vec<3, T> const& ray_dir, vec<3, T> const& cyl_origin, vec<3, T> const& cyl_dir, T cyl_radius, T cyl_height);

		static T iraycast(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction, vec<3, T> const& point);

		// TODO: test this:
		static vec<3, T> project_point_to_ray(vec<3, T> const& point, vec<3, T> const& ray_origin, vec<3, T> const& ray_direction);
	};

	//using raycast2d = Raycast2D<real>;
	//using draycast2d = Raycast2D<r64>;
	//using fraycast2d = Raycast2D<r32>;
	//
	//using raycast3d = Raycast3D<real>;
	//using draycast3d = Raycast3D<r64>;
	//using fraycast3d = Raycast3D<r32>;

#if 0
	struct NIGHT_API Ray3D
	{
		vec3 origin{ vec3(0, 0, 0) };
		vec3 direction{ vec3(0, 0, 0) };
	};

	struct NIGHT_API Ray2D
	{
		vec2 origin{ vec2(0, 0) };
		vec2 direction{ vec2(0, 0) };
	};

	struct NIGHT_API raycast
	{
		struct Result2D
		{
			u8 result;

			union
			{
				array<real, 2> thetas;
				struct
				{
					real t0;
					real t1;
				};
				real t;
			};

			union
			{
				array<vec2, 2> normals;
				struct
				{
					vec2 n0;
					vec2 n1;
				};
				vec2 normal;
			};

			template<s32 T = 0>
			vec2 contact(vec2 const& ray_origin, vec2 const& ray_direction) const
			{
				if constexpr (T == 0)
				{
					return ray_origin + ray_direction * t0;
				}
				else
				{
					return ray_origin + ray_direction * t1;
				}
			}
		};

		struct Result3D
		{
			u8 result;

			union
			{
				array<real, 2> thetas;
				struct
				{
					real t0;
					real t1;
				};
				real t;
			};

			union
			{
				array<vec3, 2> normals;
				struct
				{
					vec3 n0;
					vec3 n1;
				};
				vec3 normal;
			};

			template<s32 T = 0>
			vec3 contact(vec3 const& ray_origin, vec3 const& ray_direction) const
			{
				if constexpr (T == 0)
				{
					return ray_origin + ray_direction * t0;
				}
				else
				{
					return ray_origin + ray_direction * t1;
				}
			}
		};

		static Result2D plane(const vec2& ray_origin, const vec2& ray_dir, const vec2& plane_origin, const vec2& plane_normal, real epsilon = 0.0f);

		static Result3D plane(const vec3& ray_origin, const vec3& ray_dir, const vec3& plane_origin, const vec3& plane_normal, real epsilon = 0.0f); // TODO: better implement this.

		static Result2D line(vec2 const& ray_origin, vec2 const& ray_dir, vec2 const& p1, vec2 const& p2, real epsilon = 0.0f);

		static Result2D circle(const vec2& ray_origin, const vec2& ray_direction, const vec2& circle_origin, real circle_radius);

		static Result3D sphere(const vec3& ray_origin, const vec3& ray_direction, const vec3& sphere_origin, real sphere_radius);

		template<typename T>
		inline static real get_ray_t(T const& point, T const& ray_origin, T const& ray_direction)
		{
			T p2 = ray_origin + ray_direction;
			T const& p1 = ray_origin;
			return math::dot((p2 - p1) / length(p2 - p1), (point - p1) / length(p2 - p1));
		}

		static vec3 project_point_to_ray(const vec3& point, const vec3& ray_origin, const vec3& ray_direction);

		struct TriangleResult3D
		{
			//u8 result;
			u8 result()
			{
				return (coordinate.x >= 0.0f && coordinate.y >= 0.0f && coordinate.z >= 0.0f);
			}

			real t{INFINITY};
			vec3 coordinate{ vec3(-INFINITY) };
			vec3 normal{ vec3(0) };

			vec3 contact(vec3 const& ray_origin, vec3 const& ray_direction) const
			{
				return ray_origin + ray_direction * t;
			}
		};

		static TriangleResult3D triangle(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& p1, vec3 const& p2, vec3 const& p3, real epsilon = 0.0f);

		// static Result2D convex(vec2 const& ray_origin, vec2 const& ray_dir, vector<vec2> const& convex);

		static Result3D cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, vec3 const& cyl_dir, real cyl_radius, real cyl_height);
		//static Result3D infinite_cylinder(vec3 const& ray_origin, vec3 const& ray_dir, vec3 const& cyl_origin, real cyl_radius, vec3 const& cyl_dir);
	};
#endif

	template<typename T>
	inline RaycastResult2D<T> Raycast2D<T>::plane(vec<2, T> const& ray_origin, vec<2, T> const& ray_dir, vec<2, T> const& plane_origin, vec<2, T> const& plane_normal, T epsilon)
	{
		RaycastResult2D<T> result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		T d2 = Math<T>::dot(ray_dir, plane_normal);
		if (abs(d2) <= epsilon)
		{
			return result;
		}

		T d1 = Math<T>::dot(plane_origin - ray_origin, plane_normal);
		result.result = true;
		result.t0 = d1 / d2;
		result.normal = plane_normal;
		return result;
	}

	template<typename T>
	inline RaycastResult2D<T> Raycast2D<T>::line(vec<2, T> const& ray_origin, vec<2, T> const& ray_dir, vec<2, T> const& p1, vec<2, T> const& p2, T epsilon)
	{
		// TODO: this function is very bad.
		vec<2, T> line_direction = p2 - p1;
		vec<2, T> ray_normal = { -ray_dir.y, ray_dir.x };
		vec<2, T> line_normal = { -line_direction.y, line_direction.x };

		RaycastResult2D<T> rc1 = plane(ray_origin, ray_dir, p1, line_normal, epsilon);
		RaycastResult2D<T> rc2 = plane(p1, line_direction, ray_origin, ray_normal, epsilon);

		rc1.normal = line_normal;

		if (rc2.t < 0.0f || rc2.t > 1.0f)
		{
			rc1.result = false;
		}

		return rc1;
	}

	template<typename T>
	inline RaycastResult2D<T> Raycast2D<T>::circle(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, vec<2, T> const& circle_origin, T circle_radius)
	{
		RaycastResult2D<T> result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		T a = Math<T>::dot(ray_direction, ray_direction);
		T b = 2 * Math<T>::dot(ray_direction, circle_origin - ray_origin);
		T c = Math<T>::dot(circle_origin - ray_origin, circle_origin - ray_origin);
		c -= circle_radius * circle_radius;

		T dt = b * b - 4 * a * c;

		if (dt >= 0)
		{
			T t0 = (-b - sqrt(dt)) / (a * 2);
			T t1 = (-b + sqrt(dt)) / (a * 2);

			if (t0 > t1)
			{
				SWAP(t0, t1); // TODO: remove
			}

			result.result = true;
			result.t0 = -t0; // TODO: don't know why they are inverted
			result.t1 = -t1;
			vec<2, T> c0 = ray_origin + ray_direction * result.t0;
			vec<2, T> c1 = ray_origin + ray_direction * result.t1;
			result.n0 = Math<T>::normalize(c0 - circle_origin); // TODO: can probably div by radius
			result.n1 = Math<T>::normalize(c1 - circle_origin);
			return result;
		}

		return result;
	}

	template<typename T>
	inline vec<2, T> Raycast2D<T>::project_point_to_ray(const vec<2, T>& point, const vec<2, T>& ray_origin, const vec<2, T>& ray_direction)
	{
		T t = Math<T>::dot(point - ray_origin, ray_direction) / Math<T>::dot(ray_direction, ray_direction);
		return ray_origin + ray_direction * t;
	}

	template<typename T>
	inline T Raycast2D<T>::iraycast(vec<2, T> const& ray_origin, vec<2, T> const& ray_direction, vec<2, T> const& point)
	{
		vec<2, T> p2 = ray_origin + ray_direction;
		vec<2, T> const& p1 = ray_origin;
		return Math<T>::dot((p2 - p1) / Math<T>::length(p2 - p1), (point - p1) / Math<T>::length(p2 - p1));
	}

	template<typename T>
	inline RaycastResult3D<T> Raycast3D<T>::plane(vec<3, T> const& ray_origin, vec<3, T> const& ray_dir, vec<3, T> const& plane_origin, vec<3, T> const& plane_normal, T epsilon)
	{
		RaycastResult3D<T> result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		T d2 = Math<T>::dot(ray_dir, plane_normal);
		if (abs(d2) <= epsilon)
		{
			return result;
		}

		T d1 = Math<T>::dot(plane_origin - ray_origin, plane_normal);
		result.result = true;
		result.t0 = d1 / d2;
		result.normal = d2 > 0.0f ? -plane_normal : plane_normal; // normal should always be against ray direction
		return result;
	}

	template<typename T>
	inline RaycastResult3D<T> Raycast3D<T>::sphere(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction, vec<3, T> const& sphere_origin, T sphere_radius)
	{
		RaycastResult3D<T> result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		T a = Math<T>::dot(ray_direction, ray_direction);
		T b = 2 * Math<T>::dot(ray_direction, sphere_origin - ray_origin);
		T c = Math<T>::dot(sphere_origin - ray_origin, sphere_origin - ray_origin);
		c -= sphere_radius * sphere_radius;

		T dt = b * b - 4 * a * c;

		if (dt >= 0)
		{
			T t0 = (-b - sqrt(dt)) / (a * 2);
			T t1 = (-b + sqrt(dt)) / (a * 2);

			if (t0 > t1)
			{
				SWAP(t0, t1); // TODO: remove
			}

			result.result = true;
			result.t0 = -t1; // TODO: don't know why they are inverted
			result.t1 = -t0;
			vec<3, T> c0 = ray_origin + ray_direction * result.t0;
			vec<3, T> c1 = ray_origin + ray_direction * result.t1;
			result.n0 = Math<T>::normalize(c0 - sphere_origin); // TODO: can probably div by radius
			result.n1 = Math<T>::normalize(c1 - sphere_origin);
			return result;
		}

		result.result = false;
		return result;
	}

	template<typename T>
	inline RaycastTriangleResult<T> Raycast3D<T>::triangle(vec<3, T> const& ray_origin, vec<3, T> const& ray_dir, vec<3, T> const& p1, vec<3, T> const& p2, vec<3, T> const& p3, T epsilon)
	{
		vec<3, T> n = Math<T>::cross(p2 - p1, p3 - p2);

		auto rcp = plane(ray_origin, ray_dir, p1, n, epsilon);
		if (!rcp.result)
		{
			return { .t = -INFINITY, .coordinate = vec<3, T>(-INFINITY), .normal = vec<3, T>(0) };
		}

		vec<3, T> cx = Math<T>::cross(p3 - p2, n);
		if (Math<T>::dot(cx, p3 - p1) < 0.0f)
		{
			cx = -cx;
		}

		vec<3, T> cy = Math<T>::cross(p1 - p3, n);
		if (Math<T>::dot(cy, p1 - p2) < 0.0f)
		{
			cy = -cy;
		}

		vec<3, T> cz = Math<T>::cross(p2 - p1, n);
		if (Math<T>::dot(cz, p2 - p3) < 0.0f)
		{
			cz = -cz;
		}

		RaycastTriangleResult<T> result;

		vec<3, T> contact = rcp.contact(ray_origin, ray_dir);
		result.t = rcp.t0;
		result.coordinate.x = -Math<T>::distance_to_plane(contact, p2, cx);
		result.coordinate.y = -Math<T>::distance_to_plane(contact, p3, cy);
		result.coordinate.z = -Math<T>::distance_to_plane(contact, p1, cz);
		result.normal = Math<T>::normalize(Math<T>::dot(n, ray_dir) > 0 ? -n : n); // normal should always be against ray direction

		return result;
	}

	template<typename T>
	inline RaycastResult3D<T> Raycast3D<T>::cylinder(vec<3, T> const& ray_origin, vec<3, T> const& ray_dir, vec<3, T> const& cyl_origin, vec<3, T> const& cyl_dir, T cyl_radius, T cyl_height)
	{
		// TODO: this function is bad.
		RaycastResult3D<T> result;
		result.result = false;
		result.t0 = INFINITY;
		result.t1 = -INFINITY;
		result.n0 = {};
		result.n1 = {};

		vec<3, T> cap_a = cyl_origin + cyl_dir * cyl_height;
		vec<3, T> cap_b = cyl_origin - cyl_dir * cyl_height;

		vec<3, T> AB = (cap_b - cap_a);
		vec<3, T> AO = (ray_origin - cap_a);
		vec<3, T> AOxAB = (Math<T>::cross(AO, AB));
		vec<3, T> VxAB = (Math<T>::cross(ray_dir, AB));
		T ab2 = Math<T>::dot(AB, AB);
		T A = Math<T>::dot(VxAB, VxAB);
		T B = 2 * Math<T>::dot(VxAB, AOxAB);
		T C = (Math<T>::dot(AOxAB, AOxAB) - (cyl_radius * cyl_radius * ab2));

		T discr = B * B - 4 * A * C;

		if (discr < 0)
		{
			return result;
		}

		T t1 = (-B + sqrt(discr)) / (2 * A);
		T t2 = (-B - sqrt(discr)) / (2 * A);

		if (t1 > t2)
		{
			SWAP(t1, t2);
		}

		vec<3, T> contact1 = ray_origin + ray_dir * t1;
		T c1a = Math<T>::distance_to_plane(contact1, cap_a, cyl_dir);
		T c1b = Math<T>::distance_to_plane(contact1, cap_b, -cyl_dir);
		u8 c1ax = (c1a < 0);
		u8 c1bx = (c1b < 0);

		T min_t = INFINITY;
		if (c1ax && c1bx)
		{
			// normal is on cylinder body
			vec<3, T> pptl = project_point_to_ray(contact1, cyl_origin, cyl_dir);
			result.n0 = Math<T>::normalize(contact1 - pptl);
			min_t = t1;
		}
		else
		{
			if (!c1ax)
			{
				auto rc = plane(ray_origin, ray_dir, cap_a, cyl_dir);
				if (rc.result)
				{
					vec<3, T> contact = ray_origin + ray_dir * rc.t0;
					T d = Math<T>::length(contact - cap_a);
					if (d < cyl_radius)
					{
						result.n0 = rc.normal; // normal of t0 is cap a normal
						min_t = MIN(min_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge case");
				}
			}
			if (!c1bx)
			{
				auto rc = plane(ray_origin, ray_dir, cap_b, -cyl_dir);
				if (rc.result)
				{
					vec<3, T> contact = ray_origin + ray_dir * rc.t0;
					T d = Math<T>::length(contact - cap_b);
					if (d < cyl_radius)
					{
						result.n0 = rc.normal; // normal of t0 is cap b normal
						min_t = MIN(min_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge");
				}
			}
		}

		vec<3, T> contact2 = ray_origin + ray_dir * t2;
		T c2a = Math<T>::distance_to_plane(contact2, cap_a, cyl_dir);
		T c2b = Math<T>::distance_to_plane(contact2, cap_b, -cyl_dir);
		u8 c2ax = (c2a < 0);
		u8 c2bx = (c2b < 0);
		T max_t = -INFINITY;

		if (c2ax && c2bx)
		{
			// normal is on cylinder body
			vec<3, T> pptl = project_point_to_ray(contact1, cyl_origin, cyl_dir);
			result.n1 = Math<T>::normalize(contact1 - pptl);
			max_t = t2;
		}
		else
		{
			if (!c2ax)
			{
				auto rc = plane(ray_origin, ray_dir, cap_a, cyl_dir);
				if (rc.result)
				{
					vec<3, T> contact = ray_origin + ray_dir * rc.t0;
					T d = Math<T>::length(contact - cap_a);
					if (d < cyl_radius)
					{
						result.n1 = rc.normal; // normal of t1 is cap a normal
						max_t = MAX(max_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge case");
				}
			}
			if (!c2bx)
			{
				auto rc = plane(ray_origin, ray_dir, cap_b, -cyl_dir);
				if (rc.result)
				{
					vec<3, T> contact = ray_origin + ray_dir * rc.t0;
					T d = Math<T>::length(contact - cap_b);
					if (d < cyl_radius)
					{
						result.n1 = rc.normal; // normal of t1 is cap b normal
						max_t = MAX(max_t, rc.t0);
					}
				}
				else
				{
					//WARNING("TODO: handle edge case");
				}
			}
		}

		result.result = min_t != INFINITY;
		result.t0 = min_t;
		result.t1 = max_t;
		// TODO: handle normals

		return result;
	}

	template<typename T>
	inline T Raycast3D<T>::iraycast(vec<3, T> const& ray_origin, vec<3, T> const& ray_direction, vec<3, T> const& point)
	{
		vec<3, T> p2 = ray_origin + ray_direction;
		vec<3, T> const& p1 = ray_origin;
		return Math<T>::dot((p2 - p1) / Math<T>::length(p2 - p1), (point - p1) / Math<T>::length(p2 - p1));
	}

	template<typename T>
	inline vec<3, T> Raycast3D<T>::project_point_to_ray(vec<3, T> const& point, vec<3, T> const& ray_origin, vec<3, T> const& ray_direction)
	{
		T t = Math<T>::dot(point - ray_origin, ray_direction) / Math<T>::dot(ray_direction, ray_direction);
		return ray_origin + ray_direction * t;
	}

}