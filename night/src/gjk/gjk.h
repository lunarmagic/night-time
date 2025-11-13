#pragma once

//#include "log/log.h"
#include "math/math.h"
#include "raycast/raycast.h"
#include "debug_renderer/DebugRenderer.h"

#define NIGHT_GJK_DEFAULT_EPSILON 0.0001f
#define NIGHT_GJK_DEFAULT_MAX_ITERATIONS 32

namespace night
{
	template<typename T = real>
	struct ShapeCastResult2D
	{
		u8 result;

		union
		{
			array<T, 2> thetas;
			struct
			{
				T t0;
				T t1;
			};
		};

		union
		{
			array<vec<2, T>, 2> normals;
			struct
			{
				vec<2, T> n0;
				vec<2, T> n1;
			};
		};

		// TODO: add contact points
	};

	template<typename T = real>
	struct ShapeCastResult3D
	{
		u8 result;

		union
		{
			array<T, 2> thetas;
			struct
			{
				T t0;
				T t1;
			};
		};

		union
		{
			array<vec<3, T>, 2> normals;
			struct
			{
				vec<3, T> n0;
				vec<3, T> n1;
			};
		};

		// TODO: add contact points
	};

	template<typename T = real>
	struct IntersectsParams2D
	{
		function<vec<2, T>(vec<2, T> const&)> support_a;
		function<vec<2, T>(vec<2, T>)> support_b;
		s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS;
	};

	template<typename T = real>
	struct ShapeCastParams2D : public IntersectsParams2D<T>
	{
		vec<2, T> motion = {};
		function<vec<2, T>(vec<2, T> const&)> support_casted;
		function<vec<2, T>(vec<2, T>)> support_against;
		s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS;
		T epsilon = NIGHT_GJK_DEFAULT_EPSILON;
	};

	template<typename T = real>
	struct IntersectsParams3D
	{
		function<vec<3, T>(vec<3, T> const&)> support_a;
		function<vec<3, T>(vec<3, T> const&)> support_b;
		s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS;
	};

	template<typename T = real>
	struct ShapeCastParams3D
	{
		vec<3, T> motion = {};
		function<vec<3, T>(vec<3, T> const&)> support_casted;
		function<vec<3, T>(vec<3, T> const&)> support_against;
		s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS;
		T epsilon = NIGHT_GJK_DEFAULT_EPSILON;
	};

	template<typename T = real>
	struct GJK2D
	{
		// TODO: return simplex
		static u8 intersects(ShapeCastParams2D<T> const& params);
		static ShapeCastResult2D<T> shape_cast(ShapeCastParams2D<T> const& params, u8 skip_t1 = false);
		
		template<typename _It>
		static vec<2, T> support_polygon(vec<2, T> const& direction, _It begin, _It end);
		static vec<2, T> support_circle(vec<2, T> const& direction, vec<2, T> origin, T radius);

	private:

		//static ShapeCastResult2D<T> shape_cast_impl(ShapeCastParams2D<T> const& params, u8 skip_t1);
	};

	//using gjk2d = GJK2D<real>;
	//using dgjk2d = GJK2D<r64>;
	//using fgjk2d = GJK2D<r32>;

	template<typename T = real>
	struct GJK3D
	{
		// TODO: return simplex
		static u8 intersects(IntersectsParams3D<T> const& params);
		static ShapeCastResult3D<T> shape_cast(ShapeCastParams3D<T> const& params, u8 skip_t1 = false);

		template<typename _It>
		static vec<3, T> support_polygon(vec<3, T> const& direction, mat<4, 4, T> const& transform, _It begin, _It end);
		static vec<3, T> support_sphere(vec<3, T> const& direction, vec<3, T> const& origin, T radius);
		static vec<3, T> support_cylinder(vec<3, T> const& direction, mat<4, 4, T> const& transform, T radius, T height);
		static vec<3, T> support_cone(vec<3, T> const& direction, mat<4, 4, T> const& transform, T radius, T height);

	private:

		static ShapeCastResult3D<T> shape_cast_impl(ShapeCastParams3D<T> const& params, u8 skip_t1);
	};

	//using gjk3d = GJK3D<real>;
	//using dgjk3d = GJK3D<r64>;
	//using fgjk3d = GJK3D<r32>;

#if 0
	struct ShapeCastResult3D
	{
		u8 result;

		union
		{
			array<r64, 2> thetas;
			struct
			{
				r64 t0;
				r64 t1;
			};
		};

		union
		{
			array<dvec3, 2> normals;
			struct
			{
				dvec3 n0;
				dvec3 n1;
			};
		};
	};

	struct ShapeCastParams3D
	{
		dvec3 motion;
		function<dvec3(dvec3 const&)> support_casted;
		function<dvec3(dvec3)> support_against;
		r64 epsilon = 0.0001f;
		s32 max_iterations = 32;
	};

	// TODO: template this singleton
	// TODO: add support functions for convex, sphere, cylinder, and cone
	struct NIGHT_API gjk
	{
		static u8 intersects(function<dvec2(dvec2 const&)> const& support_a, function<dvec2(dvec2 const&)> const& support_b, r64 epsilon = NIGHT_GJK_DEFAULT_EPSILON, s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS);
		static u8 intersects(function<dvec3(dvec3 const&)> const& support_a, function<dvec3(dvec3 const&)> const& support_b, r64 epsilon = NIGHT_GJK_DEFAULT_EPSILON, s32 max_iterations = NIGHT_GJK_DEFAULT_MAX_ITERATIONS);

		static ShapeCastResult3D shape_cast(ShapeCastParams3D const& params, u8 skip_t1 = false);

	private:

		static ShapeCastResult3D shape_cast_impl(ShapeCastParams3D const& params, u8 skip_t1);
	};
#endif
	template<typename T>
	inline u8 GJK2D<T>::intersects(ShapeCastParams2D<T> const& params)
	{
		ERROR("TODO: implement this function");
		return u8();
	}

	template<typename T>
	inline ShapeCastResult2D<T> GJK2D<T>::shape_cast(ShapeCastParams2D<T> const& params, u8 skip_t1)
	{
		ERROR("TODO: implement this function");
		return ShapeCastResult2D<T>();
	}

	template<typename T>
	inline vec<2, T> GJK2D<T>::support_circle(vec<2, T> const& direction, vec<2, T> origin, T radius)
	{
		ERROR("TODO: implement this function");
		return vec<2, T>();
	}

	template<typename T>
	template<typename _It>
	inline vec<2, T> GJK2D<T>::support_polygon(vec<2, T> const& direction, _It begin, _It end)
	{

		ERROR("TODO: implement this function");
		return vec<2, T>();
	}

	template<typename T>
	inline u8 GJK3D<T>::intersects(IntersectsParams3D<T> const& params)
	{
		auto support_m = [&](const vec<3, T>& direction) -> vec<3, T>
			{
				vec<3, T> supa = params.support_a(direction);
				vec<3, T> supb = params.support_b(-direction);
				return supa - supb;
			};

		vec<3, T> simplex[4];
		s32 simplex_count{ 0 };

		vec<3, T> direction = LEFT;
		vec<3, T> opposite_direction = -direction;
		vec<3, T> perpendicular_direction = Math<T>::perp(direction);

		simplex[0] = support_m(direction);
		simplex[1] = support_m(opposite_direction);
		simplex[2] = support_m(perpendicular_direction);
		simplex_count = 3;

		constexpr vec<3, T> origin = (vec<3, T>)ORIGIN;

		for (s32 i = 0; i < params.max_iterations; i++)
		{
			switch (simplex_count)
			{
			case 3: // triangle case
			{
				vec<3, T> ab = simplex[1] - simplex[0];
				vec<3, T> ac = simplex[2] - simplex[0];
				vec<3, T> ao = origin - simplex[0];

				// TODO: get winding order right, don't check distance.
				direction = Math<T>::normalize(Math<T>::cross(ab, ac));
				if (Math<T>::dot(direction, origin - simplex[0]) < 0.0f)
				{
					direction = -direction;
				}

				simplex[3] = support_m(direction);

				if (Math<T>::dot(simplex[3], direction) < 0)
				{
					return false;
				}

				simplex_count = 4;

				break;
			}

			case 4: // pyramid case
			{
				vec<3, T> const& a = simplex[0];
				vec<3, T> const& c = simplex[1];
				vec<3, T> const& b = simplex[2];
				vec<3, T> const& d = simplex[3];

				vec<3, T> da = a - d;
				vec<3, T> db = b - d;
				vec<3, T> dc = c - d;
				vec<3, T> d_o = origin - d;

				// TODO: make sure winding order is currect, don't check dot.
				vec<3, T> dab_perp = Math<T>::cross(da, db);
				if (Math<T>::dot(dab_perp, dc) > 0.0f)
				{
					dab_perp = -dab_perp;
				}

				vec<3, T> dbc_perp = Math<T>::cross(db, dc);
				if (Math<T>::dot(dbc_perp, da) > 0.0f)
				{
					dbc_perp = -dbc_perp;
				}

				vec<3, T> dca_perp = Math<T>::cross(dc, da);
				if (Math<T>::dot(dca_perp, db) > 0.0f)
				{
					dca_perp = -dca_perp;
				}

				T dot_dab = Math<T>::dot(dab_perp, d_o);
				T dot_dbc = Math<T>::dot(dbc_perp, d_o);
				T dot_dca = Math<T>::dot(dca_perp, d_o);

				// TODO: optimize:
				if (dot_dab < 0.0f && dot_dbc < 0.0f && dot_dca < 0.0f)
				{
					return true;
				}

				if (dot_dab > dot_dbc && dot_dab > dot_dca && dot_dab > 0.0f)
				{
					simplex[0] = a;
					simplex[1] = b;
					simplex[2] = d;
					simplex_count = 3;
					continue;
				}
				else if (dot_dbc > dot_dab && dot_dbc > dot_dca && dot_dbc > 0.0f)
				{
					simplex[0] = b;
					simplex[1] = c;
					simplex[2] = d;
					simplex_count = 3;
					continue;
				}
				else if (dot_dca > dot_dab && dot_dca > dot_dbc && dot_dca > 0.0f)
				{
					simplex[0] = a;
					simplex[1] = c;
					simplex[2] = d;
					simplex_count = 3;
					continue;
				}
			}
			}
		}

		return false;
	}

	template<typename T>
	inline ShapeCastResult3D<T> GJK3D<T>::shape_cast(ShapeCastParams3D<T> const& params, u8 skip_t1)
	{
		// TODO: better integrate inverse into algorithm.
		ShapeCastResult3D<T> forward;
		ShapeCastResult3D<T> inverse;
		ShapeCastParams3D<T> iparams = params;
		iparams.motion = -iparams.motion;

		{
			DB_ALGO_SCOPED("GJK Shapecast");
			forward = shape_cast_impl(iparams, skip_t1);
		}

		forward.t0 = -forward.t0;

		ShapeCastResult3D<T> combined;

		combined.t0 = forward.t0;
		combined.n0 = forward.n0;

		if (!skip_t1 && forward.result)
		{
			DB_ALGO_SCOPED("GJK Inverse Shapecast");
			inverse = shape_cast_impl(params, skip_t1);
			combined.result = (forward.result && inverse.result);
			combined.t1 = inverse.t0;
			combined.n1 = inverse.n0;
		}
		else
		{
			combined.result = forward.result;
			combined.t1 = INFINITY;
			combined.n1 = vec<3, T>(0);
		}

		return combined;
	}

#define NIGHT_GJK_SHAPECAST_PRECISE_ESPILON 0.000001
#define NIGHT_DB_DRAW_CSO_RESOLUTION 10
	template<typename T>
	inline ShapeCastResult3D<T> GJK3D<T>::shape_cast_impl(ShapeCastParams3D<T> const& params, u8 skip_t1)
	{
		ShapeCastResult3D<T> result;
		result.t0 = INFINITY;
		result.n0 = vec<3, T>(0);
		result.result = false;

		auto support_cso = [DB_ALGO_LAMBDA_CAPTURE](const vec<3, T>& direction) -> vec<3, T>
			{
				vec<3, T> supa = params.support_casted(direction);
				vec<3, T> supb = params.support_against(-direction);
				return supa - supb;
			};

		vec<3, T> motion = -params.motion;
		vec<3, T> origin = ORIGIN;

		auto db_fn = [=]()
			{
				// TODO: handle the problem of calling this many times
				uset<vec<3, T>> cso_points;

				vec<3, T> const& direction = RIGHT;
				vec<3, T> const& i_axis = FORWARD;
				vec<3, T> const& j_axis = UP;

				for (s32 i = 0; i < NIGHT_DB_DRAW_CSO_RESOLUTION; i++)
				{
					T ti = ((T)i / (T)(NIGHT_DB_DRAW_CSO_RESOLUTION - 1)) * R_PI / 2;
					mat<4, 4, T> rot_i = Math<T>::rotate(ti, i_axis);
					vec<3, T> i_direction = rot_i * vec<4, T>(direction, 1);

					for (s32 j = 0; j < NIGHT_DB_DRAW_CSO_RESOLUTION; j++)
					{
						T tj = ((T)j / (T)(NIGHT_DB_DRAW_CSO_RESOLUTION - 1)) * R_PI / 2;
						mat<4, 4, T> rot_j = Math<T>::rotate(tj, j_axis);
						vec<3, T> j_direction = rot_j * vec<4, T>(i_direction, 1);

						array<vec<3, T>, 8> directions;

						directions[0] = j_direction;
						directions[1] = { j_direction.x, -j_direction.y, j_direction.z };
						directions[2] = { -j_direction.x, -j_direction.y, j_direction.z };
						directions[3] = { -j_direction.x, j_direction.y, j_direction.z };

						directions[4] = { j_direction.x, j_direction.y, -j_direction.z };
						directions[5] = { j_direction.x, -j_direction.y, -j_direction.z };
						directions[6] = { -j_direction.x, -j_direction.y, -j_direction.z };
						directions[7] = { -j_direction.x, j_direction.y, -j_direction.z };

						for (const auto& k : directions)
						{
							vec<3, T> point = support_cso(k);
							cso_points.insert(point);
						}
					}
				}

				for (const auto& i : cso_points)
				{
					DB_ALGO_DRAW_POINT((vec3)i, LIGHT_BLUE);
				}
			};
		DB_ALGO_DRAW_POINT(ORIGIN, RED);
		DB_ALGO_DRAW_LINE(ORIGIN, (vec3)-motion * 1000.0f, RED.opaqued(0.5f));
		DB_ALGO_DRAW_FN(db_fn);
		DB_ALGO_SCOPED("Shapecast Iterations");

		array<vec<3, T>, 4> simplex = {};
		vec<3, T>& a = simplex[0];
		vec<3, T>& b = simplex[1];
		vec<3, T>& c = simplex[2];
		vec<3, T>& w = simplex[3];

		vec<3, T> search_a = Math<T>::perp(motion);
		vec<3, T> search_b = -search_a;
		a = support_cso(search_a);
		b = support_cso(search_b);

		DB_ALGO_INCREMENT_STEP();
		DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);

		for (s32 i = 0; i < params.max_iterations; i++)
		{
			// line case:
			{
				vec<3, T> ab = b - a;
				vec<3, T> n = a - -motion * (T)1000; // TODO: fix very stupid solution.
				vec<3, T> abdir = Math<T>::cross(ab, n);

				// epsilon error
				T d = Math<T>::dot(abdir, abdir);
				if (d < NIGHT_GJK_SHAPECAST_PRECISE_ESPILON)
				{
					break;
				}

				vec<3, T> abc = Math<T>::cross(abdir, ab);
				vec<3, T> normal = Math<T>::normalize(abc);
				vec<3, T> projected = Math<T>::dot(n, normal) / Math<T>::dot(normal, motion) * motion;
				vec<3, T> projected_normal = Math<T>::cross(ab, projected - n);

				vec<3, T> search_direction;

				T d2 = Math<T>::dot(projected_normal, projected_normal);
				if (d2 < NIGHT_GJK_SHAPECAST_PRECISE_ESPILON)
				{
					T t = Math<T>::dot(projected - n, ab) / Math<T>::dot(ab, ab);
					if (t >= 0 && t <= 1)
					{
						// triangle edge lies near ray, continue as if it was a hit
						search_direction = Math<T>::cross(normal, ab);
						c = support_cso(search_direction);

						// double raycast:
						T t0 = INFINITY;
						vec<3, T> n0 = {};
						u8 r = false;

						{
							T d2 = Math<T>::dot(motion, normal);
							if (d2 == 0.0f)
							{
								r = false;
							}
							else
							{

							}
							T d1 = Math<T>::dot(a - origin, normal);
							r = true;
							t0 = d1 / d2;
							n0 = normal;
						}

						if (r)
						{
							result.t0 = t0;
							result.n0 = n0;
							result.result = true;
						}

						DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);
						DB_ALGO_INCREMENT_STEP();

#ifdef NIGHT_DBAR
						{
							DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
							DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
							DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);
							DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);
							if (Math<T>::dot(Math<T>::cross(c - a, b - a), motion) > 0)
							{
								SWAP(a, b);
							}
						}
#endif

						break;
					}
					else
					{
						search_direction = Math<T>::cross(normal, ab);
						c = support_cso(search_direction);
					}
				}
				else
				{
					search_direction = Math<T>::cross(projected_normal, ab);
					c = support_cso(search_direction);
				}

				DB_ALGO_INCREMENT_STEP();
				DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
				DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);

				// correct winding order:
				if (Math<T>::dot(Math<T>::cross(c - a, b - a), motion) > 0)
				{
					SWAP(a, b);
				}
			}

			// triangle case:
			{

				DB_ALGO_INCREMENT_STEP();
				{
					DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
					DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
					DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);
					DB_ALGO_DRAW_ARROW(((vec3)a + (vec3)b + (vec3)c) / 3.0f, (vec3)Math<T>::normalize(Math<T>::cross(b - a, c - a)), ORANGE);
				}

				// epsilon error: the triangle is a thin line.
				vec<3, T> abc = Math<T>::cross(b - a, c - a);
				T d = Math<T>::dot(abc, abc);
				if (d < params.epsilon + NIGHT_GJK_SHAPECAST_PRECISE_ESPILON)
				{
					break;
				}

				// epsilon error: the triangle tangent with the motion, or facing away from the motion
				if (Math<T>::dot(abc, motion) < NIGHT_GJK_SHAPECAST_PRECISE_ESPILON)
				{
					break;
				}

				// TODO: optimize out triangle raycast
				auto rct = Raycast3D<>::triangle(ORIGIN, motion, a, b, c);

				if (rct.result())
				{
					DB_ALGO_DRAW_POINT(rct.contact(ORIGIN, (vec3)motion), RED);
					result.t0 = rct.t;
					result.n0 = rct.normal;
					result.result = true;
					break;
				}

				if (rct.coordinate.x < rct.coordinate.y)
				{
					if (rct.coordinate.x < rct.coordinate.z)
					{
						a = b;
						b = c;
					}
				}
				else if (rct.coordinate.y < rct.coordinate.z)
				{
					b = c;
				}
			}
		}

		if (!result.result)
		{
			return result;
		}

		// inch tetrahedron towards -motion
		for (s32 i = 0; i < params.max_iterations; i++)
		{
			// triangle case:
			{
				if (Math<T>::dot(Math<T>::cross(c - a, b - a), motion) > 0) // TODO: remove.
				{
					SWAP(a, b);
				}

				vec<3, T> n = Math<T>::normalize(Math<T>::cross(b - a, c - b));

				DB_ALGO_INCREMENT_STEP();
				{
					DB_ALGO_DRAW_ARROW(((vec3)a + (vec3)b + (vec3)c) / 3.0f, (vec3)n, ORANGE);
					DB_ALGO_DRAW_ARROW(origin, n, PURPLE);
					DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
					DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
					DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);
				}

				w = support_cso(n);

				// new support point lies within epsilon distance to the plane of the triangle, 
				// the algorithm is finished. return the triangle's raycast.
				if (Math<T>::dot(n, w - a) < params.epsilon)
				{
					break;
				}
			}

			// tetrahedron case:
			DB_ALGO_INCREMENT_STEP();
			{
				DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN.opaqued(0.75f));
				DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN.opaqued(0.75f));
				DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN.opaqued(0.75f));

				DB_ALGO_DRAW_LINE((vec3)a, (vec3)w, CYAN);
				DB_ALGO_DRAW_LINE((vec3)b, (vec3)w, CYAN);
				DB_ALGO_DRAW_LINE((vec3)c, (vec3)w, CYAN);
			}

			T max_coord = -INFINITY;
			T mc_t = INFINITY;
			vec<3, T> mc_normal = vec<3, T>(0);
			vec<3, T> mc_p1 = {};
			vec<3, T> mc_p2 = {};

			for (s32 j = 0; j < 3; j++)
			{
				vec<3, T> const& p1 = simplex[j];
				vec<3, T> const& p2 = simplex[(j + 1) % 3];

				vec<3, T> n = Math<T>::normalize(Math<T>::cross(p2 - p1, w - p2));
				DB_ALGO_DRAW_ARROW(((vec3)p1 + (vec3)p2 + (vec3)w) / 3.0f, (vec3)n, ORANGE);

				// skip backfacing triangles.
				T d = Math<T>::dot(motion, n);
				if (d <= NIGHT_GJK_SHAPECAST_PRECISE_ESPILON)
				{
					continue;
				}

				auto rct = Raycast3D<>::triangle(origin, motion, p1, p2, w);
				T coord = MIN(rct.coordinate.x, rct.coordinate.y);
				if (coord > max_coord)
				{
					max_coord = coord;
					mc_t = rct.t;
					mc_normal = rct.normal;
					mc_p1 = p1;
					mc_p2 = p2;
				}
			}

			a = mc_p1;
			b = mc_p2;
			c = w;

			DB_ALGO_DRAW_POINT((vec3)origin + (vec3)motion * (real)mc_t, RED);

			// correct winding order
			if (Math<T>::dot(Math<T>::cross(c - a, b - a), motion) > 0)
			{
				SWAP(a, b);
			}

			vec<3, T> abc = Math<T>::cross(b - a, c - a);
			if (Math<T>::dot(abc, abc) < NIGHT_GJK_SHAPECAST_PRECISE_ESPILON)
			{
				break;
			}

			result.t0 = mc_t;
			result.n0 = mc_normal;
		}

		DB_ALGO_INCREMENT_STEP();
		DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
		DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
		DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);

		return result;
	}

	template<typename T>
	template<typename _It>
	inline vec<3, T> GJK3D<T>::support_polygon(vec<3, T> const& direction, mat<4, 4, T> const& transform, _It begin, _It end)
	{
		vec<3, T> dir_normalized = Math<T>::normalize(direction); // TODO: remove
		vec<3, T> result = vec<3, T>(0);
		T max_dot = -INFINITY;

		for (auto i = begin; i != end; i++)
		{
			vec<3, T> point = (vec<3, T> const&)(*i);

			point = vec3(transform * vec4(point, 1));

			T new_d = Math<T>::dot(dir_normalized, point);
			if (new_d > max_dot)
			{
				max_dot = new_d;
				result = point;
			};
		}

		return result;
	}

	template<typename T>
	inline vec<3, T> GJK3D<T>::support_sphere(vec<3, T> const& direction, vec<3, T> const& origin, T radius)
	{
		return origin + Math<T>::normalize(direction) * radius;
	}
	
#define NIGHT_CYLINDER_SUPPORT_EPSILON 0.0001
	template<typename T>
	inline vec<3, T> GJK3D<T>::support_cylinder(vec<3, T> const& sdir, mat<4, 4, T> const& transform, T radius, T height)
	{
		vec<3, T> sdir_normalized = Math<T>::normalize(sdir);

		DecomposedTransform<T> decomp = Math<T>::decompose(transform);
		vec<3, T> const& cyl_origin = decomp.translation;
		vec<3, T> cyl_direction = Math<T>::normalize(decomp.rotation * ((vec<3, T>)FORWARD * decomp.scale));

		T d = Math<T>::dot(cyl_direction, sdir_normalized);
		if (abs(d) > 1.0 - NIGHT_CYLINDER_SUPPORT_EPSILON) // parallel
		{
			return (d > 0 ? cyl_direction : -cyl_direction) * height + cyl_origin;
		}

		vec<3, T> cap = (d > 0 ? cyl_direction : -cyl_direction) * height;

		T d2 = -Math<T>::dot(cyl_direction, cap);
		T distance = Math<T>::dot(cyl_direction, cap + sdir_normalized) + d2;
		vec<3, T> proj = (cap + sdir_normalized) - cyl_direction * distance;

		proj = cap + Math<T>::normalize(proj - cap) * radius;
		return cyl_origin + proj;
	}

#define NIGHT_CONE_SUPPORT_EPSILON 0.0001
	template<typename T>
	inline vec<3, T> GJK3D<T>::support_cone(vec<3, T> const& dir, mat<4, 4, T> const& transform, T radius, T height)
	{
		// TODO: transform vertices
		DecomposedTransform<T> decomp = Math<T>::decompose(transform);
		vec<3, T> const& cone_origin = decomp.translation;
		vec<3, T> cone_direction = Math<T>::normalize(decomp.rotation * ((vec<3, T>)FORWARD * decomp.scale));
	
		vec<3, T> dir_normalized = Math<T>::normalize(dir);
		T d = Math<T>::dot(cone_direction, dir_normalized);
		if (abs(d) > 1.0 - NIGHT_CONE_SUPPORT_EPSILON) // parallel
		{
			return (d > 0.0 ? cone_direction : -cone_direction) * height + cone_origin;
		}

		vec<3, T> tip = cone_direction * height;
		vec<3, T> base = -cone_direction * height;

		// TODO: there could be an epsilon error if dir_normalized is close to cyl_direction
		vec<3, T> proj = Math<T>::project_point_to_plane(base + dir_normalized, base, cone_direction).point;
		proj = base + Math<T>::normalize(proj - base) * radius;
		T dp = Math<T>::dot(dir_normalized, proj);
		T dt = Math<T>::dot(dir_normalized, tip);

		return cone_origin + (dp > dt ? proj : tip);
	}
	
	
	//template<typename T>
	//inline vec<3, T> GJK3D<T>::support_cone(vec<3, T> const& dir, vec<3, T> const& origin, vec<3, T> const& direction, T radius, T height)
	//{
	//	vec<3, T> dir_normalized = Math<T>::normalize(dir);
	//	T d = Math<T>::dot(direction, dir_normalized);
	//	if (abs(d) > 1.0 - NIGHT_CONE_SUPPORT_EPSILON) // parallel
	//	{
	//		return (d > 0.0 ? direction : -direction) * height + origin;
	//	}
	//
	//	vec<3, T> tip = direction * height;
	//	vec<3, T> base = -direction * height;
	//
	//	T d2 = -Math<T>::dot(-direction, base);
	//	T distance = Math<T>::dot(-direction, base + dir_normalized) + d2;
	//	vec<3, T> proj = (base + dir_normalized) - direction * distance;
	//	//vec<3, T> proj = Math<T>::project_point_to_plane(base + dir_normalized, base, direction).point;
	//	proj = base + Math<T>::normalize(proj - base) * radius;
	//	T dp = Math<T>::dot(dir_normalized, proj);
	//	T dt = Math<T>::dot(dir_normalized, tip);
	//
	//	return origin + (dp > dt ? proj : tip);
	//}
}

namespace std
{

	// TODO: figure out hash collisions

	template<>
	struct hash<night::vec<3, night::r64>>
	{
		uint64_t operator()(night::vec<3, night::r64> const& key) const
		{
			return hash<uint64_t>()(*((uint32_t*)&key.x) ^ *((uint32_t*)&key.y) ^ *((uint32_t*)&key.z));
		}
	};

	template<>
	struct hash<night::vec<3, night::r32>>
	{
		uint64_t operator()(night::vec<3, night::r32> const& key) const
		{
			return hash<uint64_t>()(*((uint32_t*)&key.x) ^ *((uint32_t*)&key.y) ^ *((uint32_t*)&key.z));
		}
	};
};