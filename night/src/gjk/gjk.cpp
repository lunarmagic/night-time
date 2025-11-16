
#include "nightpch.h"
#include "gjk.h"
#if 0
#include "math/Math.h"
//#include "profiler/Profiler.h"
#include "debug_renderer/DebugRenderer.h"
#include "color/Color.h"
#include "raycast/Raycast.h"

#define GJK_SWEEP_MAX_ITERATIONS 32

// TODO: move to math.h
namespace std
{
	template<>
	struct hash<night::dvec3>
	{
		uint64_t operator()(const night::dvec3& key) const
		{
			return hash<uint64_t>()(*((uint32_t*)&key.x) ^ *((uint32_t*)&key.y) ^ *((uint32_t*)&key.z));
		}
	};
};

namespace night
{
	u8 gjk::intersects(function<dvec2(dvec2 const&)> const& support_a, function<dvec2(dvec2 const&)> const& support_b, r64 epsilon, s32 max_iterations)
	{
		ASSERT(false); // TODO: fix this function
		DB_ALGO_SCOPED("gjk::intersects");

		auto support_m = [&](const dvec2& direction) -> dvec2
			{
				dvec2 supa = support_a(direction);
				dvec2 supb = support_b(-direction);
				return supa - supb;
			};

#ifdef NIGHT_DBAR
		DB_ALGO_INCREMENT_STEP();

		for (s32 i = 0; i < 90; i++)
		{
			real t = ((real)i / (90)) * R_PI * 2;
			dvec2 dir;
			dir.x = cos(t);
			dir.y = sin(t);
			dvec2 point = support_m(dir);
			DB_ALGO_DRAW_POINT(point, BLUE);
		}
#endif

		DB_ALGO_SCOPED("iterate simplex");

		dvec2 simplex[3];

		dvec2 direction = LEFT;
		dvec2 opposite_direction = -direction;

		simplex[0] = support_m(direction);
		simplex[1] = support_m(opposite_direction);

		s32 simplex_count = 2;

		constexpr dvec2 origin = ORIGIN;

		for (s32 i = 0; i < max_iterations; i++)
		{
			DB_ALGO_INCREMENT_STEP();

			switch (simplex_count)
			{
			case 2: // line case
			{
				DB_ALGO_DRAW_LINE(simplex[0], simplex[1], LIGHT_BLUE);
				DB_ALGO_DRAW_POINT(ORIGIN, RED);

				dvec2 ab = simplex[1] - simplex[0];
				dvec2 ao = origin - simplex[0];

				direction = dmath::normalize(dmath::triple_cross(ab, ao, ab));

				simplex[2] = support_m(direction);

				//if (dmath::dot(simplex[2], direction) < 0) // TODO: epsilon
				//{
				//	return false;
				//}

				if (simplex[2] == simplex[0] || simplex[2] == simplex[1])
				{
					return false;
				}

				simplex_count = 3;

				break;
			}

			case 3: // triangle case
			{
				DB_ALGO_DRAW_LINE(simplex[0], simplex[1], LIGHT_BLUE);
				DB_ALGO_DRAW_LINE(simplex[1], simplex[2], LIGHT_BLUE);
				DB_ALGO_DRAW_LINE(simplex[2], simplex[0], LIGHT_BLUE);
				DB_ALGO_DRAW_POINT(ORIGIN, RED);

				dvec2& c = simplex[0];
				dvec2& b = simplex[1];
				dvec2& a = simplex[2];

				dvec2 ab = b - a;
				dvec2 ac = c - a;
				dvec2 ao = origin - a;

				dvec2 ab_perp = dmath::triple_cross(ac, ab, ab);
				dvec2 ac_perp = dmath::triple_cross(ab, ac, ac);

				r64 dot_ab = dmath::dot(ab_perp, ao);
				r64 dot_ac = dmath::dot(ac_perp, ao);

				if (dot_ab > dot_ac && dot_ab > 0.0)
				{
					SWAP(b, a);
					simplex_count = 2;
				}
				else if (dot_ac > 0.0)
				{
					b = a;
					a = c;
					simplex_count = 2;
				}
				else
				{
					return true;
				}

				break;
			}
			}
		}

		return false;
	}

	// TODO: optimize this function
	u8 gjk::intersects(function<dvec3(dvec3 const&)> const& support_a, function<dvec3(dvec3 const&)> const& support_b, r64 epsilon, s32 max_iterations)
	{
		auto support_m = [&](const dvec3& direction) -> dvec3
		{
			dvec3 supa = support_a(direction);
			dvec3 supb = support_b(-direction);
			return supa - supb;
		};

		dvec3 simplex[4];
		s32 simplex_count{ 0 };

		dvec3 direction = LEFT;
		dvec3 opposite_direction = -direction;
		dvec3 perpendicular_direction = dmath::perp(direction);

		simplex[0] = support_m(direction);
		simplex[1] = support_m(opposite_direction);
		simplex[2] = support_m(perpendicular_direction);
		simplex_count = 3;

		constexpr dvec3 origin = ORIGIN;

		for (s32 i = 0; i < max_iterations; i++)
		{
			switch (simplex_count)
			{
			case 3: // triangle case
			{
				dvec3 ab = simplex[1] - simplex[0];
				dvec3 ac = simplex[2] - simplex[0];
				dvec3 ao = origin - simplex[0];

				// TODO: get winding order right, don't check distance.
				direction = dmath::normalize(dmath::cross(ab, ac));
				//if (distance_to_plane(origin, simplex[0], direction) < 0.0f)
				if(dmath::dot(direction, origin - simplex[0]) < 0.0f)
				{
					direction = -direction;
				}

				simplex[3] = support_m(direction);

				if (dmath::dot(simplex[3], direction) < 0) // TODO: epsilon
				{
					return false;
				}

				simplex_count = 4;

				break;
			}

			case 4: // pyramid case
			{
				dvec3 const& a = simplex[0];
				dvec3 const& c = simplex[1];
				dvec3 const& b = simplex[2];
				dvec3 const& d = simplex[3];

				dvec3 da = a - d;
				dvec3 db = b - d;
				dvec3 dc = c - d;
				dvec3 d_o = origin - d;

				// TODO: make sure winding order is currect, don't check dot.
				dvec3 dab_perp = dmath::cross(da, db);
				if (dmath::dot(dab_perp, dc) > 0.0f)
				{
					dab_perp = -dab_perp;
				}

				dvec3 dbc_perp = dmath::cross(db, dc);
				if (dmath::dot(dbc_perp, da) > 0.0f)
				{
					dbc_perp = -dbc_perp;
				}

				dvec3 dca_perp = dmath::cross(dc, da);
				if (dmath::dot(dca_perp, db) > 0.0f)
				{
					dca_perp = -dca_perp;
				}

				r64 dot_dab = dmath::dot(dab_perp, d_o);
				r64 dot_dbc = dmath::dot(dbc_perp, d_o);
				r64 dot_dca = dmath::dot(dca_perp, d_o);

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

#if 0
	static dvec3 project_point_to_triangle(const dvec3& point, dvec3 const& p1, dvec3 const& p2, dvec3 const& p3)
	{
		dvec3 edge0 = p2 - p1;
		dvec3 edge1 = p3 - p1;
		dvec3 v0 = p1 - point;

		float a = dmath::dot(edge0, edge0);
		float b = dmath::dot(edge0, edge1);
		float c = dmath::dot(edge1, edge1);
		float d = dmath::dot(edge0, v0);
		float e = dmath::dot(edge1, v0);

		float det = a * c - b * b;
		float s = b * e - c * d;
		float t = b * d - a * e;

		if (s + t < det)
		{
			if (s < 0.f)
			{
				if (t < 0.f)
				{
					if (d < 0.f)
					{
						s = CLAMP(-d / a, 0.f, 1.f);
						t = 0.f;
					}
					else
					{
						s = 0.f;
						t = CLAMP(-e / c, 0.f, 1.f);
					}
				}
				else
				{
					s = 0.f;
					t = CLAMP(-e / c, 0.f, 1.f);
				}
			}
			else if (t < 0.f)
			{
				s = CLAMP(-d / a, 0.f, 1.f);
				t = 0.f;
			}
			else
			{
				float invDet = 1.f / det;
				s *= invDet;
				t *= invDet;
			}
		}
		else
		{
			if (s < 0.f)
			{
				float tmp0 = b + d;
				float tmp1 = c + e;
				if (tmp1 > tmp0)
				{
					float numer = tmp1 - tmp0;
					float denom = a - 2 * b + c;
					s = CLAMP(numer / denom, 0.f, 1.f);
					t = 1 - s;
				}
				else
				{
					t = CLAMP(-e / c, 0.f, 1.f);
					s = 0.f;
				}
			}
			else if (t < 0.f)
			{
				if (a + d > b + e)
				{
					float numer = c + e - b - d;
					float denom = a - 2 * b + c;
					s = CLAMP(numer / denom, 0.f, 1.f);
					t = 1 - s;
				}
				else
				{
					s = CLAMP(-e / c, 0.f, 1.f);
					t = 0.f;
				}
			}
			else
			{
				float numer = c + e - b - d;
				float denom = a - 2 * b + c;
				s = CLAMP(numer / denom, 0.f, 1.f);
				t = 1.f - s;
			}
		}

		return p1 + s * edge0 + t * edge1;
	}

	dvec3 simplex_normal(array<dvec3, 4> const& simplex, s32 index)
	{
		dvec3 const& p1 = simplex[index];
		dvec3 const& p2 = simplex[(index + 1) % simplex.size()];
		dvec3 const& p3 = simplex[(index + 2) % simplex.size()];
		dvec3 const& w = simplex[(index + 3) % simplex.size()];
		dvec3 n = dmath::normalize(dmath::cross(p2 - p1, p3 - p2));
		if (dmath::dot(n, dmath::normalize(w - p1)) > 0.0f)
		{
			n = -n;
		}
		return n;
	}

	struct pptsr
	{
		s32 index;
		dvec3 point;
		r64 distance;
		dvec3 normal;
	};

	static pptsr project_point_to_simplex(dvec3 const& point, array<dvec3, 4> const& simplex)
	{
		// TODO: handle case if point overlapps

		pptsr min = {.distance = INFINITY};

		for (s32 i = 0; i < simplex.size(); i++)
		{
			dvec3 const& p1 = simplex[i];
			dvec3 const& p2 = simplex[(i + 1) % simplex.size()];
			dvec3 const& p3 = simplex[(i + 2) % simplex.size()];

			dvec3 proj = project_point_to_triangle(point, p1, p2, p3);
			r64 dist = distance(point, proj);
			dvec3 n = simplex_normal(simplex, i);

			if (dist == min.distance) // may need epsilon
			{
				r64 d1 = dmath::dot(min.normal, dmath::normalize(min.point - point));
				r64 d2 = dmath::dot(n, dmath::normalize(proj - point));
				if (d1 > d2)
				{
					continue;
				}
			}
			else
				if (dist < min.distance)
			{
				min.index = (i + 3) % simplex.size();
				min.point = proj;
				min.distance = dist;
				min.normal = n;
			}
		}

		return min;
	};
#endif

#define NIGHT_DB_DRAW_CSO_RESOLUTION 10
#define NIGHT_GJK_SHAPECAST_ESPILON 0.000001 // TODO: may want to increase this epsilon

	ShapeCastResult3D night::gjk::shape_cast_impl(ShapeCastParams3D const& params, u8 skip_t1)
	{
		//DB_ALGO_PUSH("GJK Shapecast"); // TODO: make different aglorithm name for the inverse shapecast

		ShapeCastResult3D result;
		result.t0 = INFINITY;
		result.n0 = dvec3(0);
		result.result = false;

		auto support_cso = [DB_ALGO_LAMBDA_CAPTURE](const dvec3& direction) -> dvec3
			{
				dvec3 supa = params.support_casted(direction);
				dvec3 supb = params.support_against(-direction);
				return supa - supb;
			};

		dvec3 motion = -params.motion;
		dvec3 origin = ORIGIN;

		//DB_ALGO_INCREMENT_STEP();
		auto db_fn = [=]()
			{
				// TODO: handle the problem of calling this many times
				uset<dvec3> cso_points;

				dvec3 const& direction = RIGHT;
				dvec3 const& i_axis = FORWARD;
				dvec3 const& j_axis = UP;

				for (s32 i = 0; i < NIGHT_DB_DRAW_CSO_RESOLUTION; i++)
				{
					r64 ti = ((r64)i / (r64)(NIGHT_DB_DRAW_CSO_RESOLUTION - 1)) * R_PI / 2;
					mat4 rot_i = dmath::rotate(ti, i_axis);
					dvec3 i_direction = rot_i * vec4(direction, 1);

					for (s32 j = 0; j < NIGHT_DB_DRAW_CSO_RESOLUTION; j++)
					{
						r64 tj = ((r64)j / (r64)(NIGHT_DB_DRAW_CSO_RESOLUTION - 1)) * R_PI / 2;
						mat4 rot_j = dmath::rotate(tj, j_axis);
						dvec3 j_direction = rot_j * vec4(i_direction, 1);

						array<dvec3, 8> directions;

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
							dvec3 point = support_cso(k);
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

		array<dvec3, 4> simplex = {};
		dvec3& a = simplex[0];
		dvec3& b = simplex[1];
		dvec3& c = simplex[2];
		dvec3& w = simplex[3];

		//dvec3 search_a = math::normalize(motion);
		//dvec3 search_b = perpendicular_vector(search_a);
		dvec3 search_a = dmath::perp(motion);
		dvec3 search_b = -search_a;
		a = support_cso(search_a);
		b = support_cso(search_b);

		//s32 simplex_size = 2;

		DB_ALGO_INCREMENT_STEP();
		DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);

		//u8 x = false;
		for (s32 i = 0; i < params.max_iterations; i++)
		{
			// line case:
			{
				dvec3 ab = b - a;
				dvec3 n = a - -motion * 1000.0; // TODO: fix very stupid solution.
				dvec3 abdir = dmath::cross(ab, n);

				// epsilon error
				r64 d = dmath::dot(abdir, abdir);
				if (d < NIGHT_GJK_SHAPECAST_ESPILON)
				{
					break;
				}

				dvec3 abc = dmath::cross(abdir, ab);
				dvec3 normal = dmath::normalize(abc);
				dvec3 projected = dmath::dot(n, normal) / dmath::dot(normal, motion) * motion;
				dvec3 projected_normal = dmath::cross(ab, projected - n);

				dvec3 search_direction;

				r64 d2 = dmath::dot(projected_normal, projected_normal);
				if (d2 < NIGHT_GJK_SHAPECAST_ESPILON)
				{
					r64 t = dmath::dot(projected - n, ab) / dmath::dot(ab, ab);
					if (t >= 0 && t <= 1)
					{
						// triangle edge lies near ray, continue as if it was a hit
						search_direction = dmath::cross(normal, ab);
						c = support_cso(search_direction);

						// double raycast:
						r64 t0 = INFINITY;
						dvec3 n0 = {};
						u8 r = false;

						{
							r64 d2 = dmath::dot(motion, normal);
							if (d2 == 0.0f)
							{
								r = false;
							}
							else
							{

							}
							r64 d1 = dmath::dot(a - origin, normal);
							r = true;
							t0 = d1 / d2;
							n0 = normal;
						}

						//auto rc = raycast::plane(origin, motion, a, normal);

						if (r)
						{
							result.t0 = t0;
							result.n0 = n0;
							result.result = true;
						}


						//simplex_size = 3;

						DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);
						DB_ALGO_INCREMENT_STEP();

#ifdef NIGHT_DBAR
						{
							DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
							DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
							DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);
							DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);
							if (dmath::dot(dmath::cross(c - a, b - a), motion) > 0)
							{
								SWAP(a, b);
							}
						}
#endif

						break;
					}
					else
					{
						search_direction = dmath::cross(normal, ab);
						c = support_cso(search_direction);
					}
				}
				else
				{
					search_direction = dmath::cross(projected_normal, ab);
					c = support_cso(search_direction);
				}

				DB_ALGO_INCREMENT_STEP();
				DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
				DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);

				// correct winding order:
				if (dmath::dot(dmath::cross(c - a, b - a), motion) > 0)
				{
					SWAP(a, b);
				}

				//simplex_size = 3;
			}

			// triangle case:
			{

				DB_ALGO_INCREMENT_STEP();
				{
					DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
					DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
					DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);
					DB_ALGO_DRAW_ARROW(((vec3)a + (vec3)b + (vec3)c) / 3.0f, (vec3)dmath::normalize(dmath::cross(b - a, c - a)), ORANGE);
				}

				// epsilon error: the triangle is a thin line.
				dvec3 abc = dmath::cross(b - a, c - a);
				r64 d = dmath::dot(abc, abc);
				if (d < params.epsilon + NIGHT_GJK_SHAPECAST_ESPILON)
				{
					break;
				}

				// epsilon error: the triangle tangent with the motion, or facing away from the motion
				if (dmath::dot(abc, motion) < NIGHT_GJK_SHAPECAST_ESPILON)
				{
					break;
				}

				// TODO: optimize out triangle raycast
				auto rct = raycast3d::triangle(ORIGIN, motion, a, b, c);

				if (rct.result())
				{
					DB_ALGO_DRAW_POINT(rct.contact(ORIGIN, motion), RED);
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

				//simplex_size = 2;
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
				if (dmath::dot(dmath::cross(c - a, b - a), motion) > 0) // TODO: remove.
				{
					SWAP(a, b);
				}

				dvec3 n = dmath::normalize(dmath::cross(b - a, c - b));

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
				//r64 d = distance_to_plane(w, a, n);
				//if (d < params.epsilon)
				if (dmath::dot(n, w - a) < params.epsilon)
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

			r64 max_coord = -INFINITY;
			r64 mc_t = INFINITY;
			dvec3 mc_normal = dvec3(0);
			dvec3 mc_p1 = {};
			dvec3 mc_p2 = {};

			for (s32 j = 0; j < 3; j++)
			{
				dvec3 const& p1 = simplex[j];
				dvec3 const& p2 = simplex[(j + 1) % 3];
				//dvec3 w2 = w;

				dvec3 n = dmath::normalize(dmath::cross(p2 - p1, w - p2));
				DB_ALGO_DRAW_ARROW(((vec3)p1 + (vec3)p2 + (vec3)w) / 3.0f, (vec3)n, ORANGE);

				// skip backfacing triangles.
				r64 d = dmath::dot(motion, n);
				if (d <= NIGHT_GJK_SHAPECAST_ESPILON)
				{
					continue;
				}

				auto rct = raycast3d::triangle(origin, motion, p1, p2, w);
				r64 coord = MIN(rct.coordinate.x, rct.coordinate.y);
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
			if (dmath::dot(dmath::cross(c - a, b - a), motion) > 0)
			{
				SWAP(a, b);
			}

			dvec3 abc = dmath::cross(b - a, c - a);
			if (dmath::dot(abc, abc) < NIGHT_GJK_SHAPECAST_ESPILON)
			{
				break;
			}

			//if (mc_t < result.t)
			//{
			result.t0 = mc_t;
			result.n0 = mc_normal;
			//}
		}

		DB_ALGO_INCREMENT_STEP();
		DB_ALGO_DRAW_LINE((vec3)a, (vec3)b, CYAN);
		DB_ALGO_DRAW_LINE((vec3)b, (vec3)c, CYAN);
		DB_ALGO_DRAW_LINE((vec3)c, (vec3)a, CYAN);

		return result;
	}

	ShapeCastResult3D gjk::shape_cast(ShapeCastParams3D const& params, u8 skip_t1)
	{
		ShapeCastResult3D forward;
		ShapeCastResult3D inverse;
		ShapeCastParams3D iparams = params;
		iparams.motion = -iparams.motion;
		{
			DB_ALGO_SCOPED("GJK Shapecast");
			forward = shape_cast_impl(iparams, skip_t1);
		}
		forward.t0 = -forward.t0;

		ShapeCastResult3D combined;
		
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
			combined.n1 = dvec3(0);
		}
		
		return combined;
	}
}
#endif