
#include "nightpch.h"
#include "gjk.h"
#include "math/math.h"
//#include "profiler/Profiler.h"
#include "debug_renderer/DebugRenderer.h"
#include "color/Color.h"
#include "raycast/raycast.h"

#define GJK_SWEEP_MAX_ITERATIONS 32

// TODO: move to math.h
namespace std
{
	template<>
	struct hash<night::vec3>
	{
		uint64_t operator()(const night::vec3& key) const
		{
			return hash<uint64_t>()(*((uint32_t*)&key.x) ^ *((uint32_t*)&key.y) ^ *((uint32_t*)&key.z));
		}
	};
};

namespace night
{
	// TODO: optimize this function
	u8 gjk::intersects(function<vec3(vec3 const&)> const& support_a, function<vec3(vec3 const&)> const& support_b, real epsilon, s32 max_iterations)
	{
		auto support_m = [&](const vec3& direction) -> vec3
		{
			vec3 supa = support_a(direction);
			vec3 supb = support_b(-direction);
			return supa - supb;
		};

		vec3 simplex[4];
		s32 simplex_count{ 0 };

		vec3 direction = LEFT;
		vec3 opposite_direction = -direction;
		vec3 perpendicular_direction = perpendicular_vector(direction);

		simplex[0] = support_m(direction);
		simplex[1] = support_m(opposite_direction);
		simplex[2] = support_m(perpendicular_direction);
		simplex_count = 3;

		constexpr vec3 origin = ORIGIN;

		for (s32 i = 0; i < max_iterations; i++)
		{
			switch (simplex_count)
			{
			case 3: // triangle case
			{
				vec3 ab = simplex[1] - simplex[0];
				vec3 ac = simplex[2] - simplex[0];
				vec3 ao = origin - simplex[0];

				// TODO: get winding order right, don't check distance.
				direction = normalize(cross(ab, ac));
				if (distance_to_plane(origin, simplex[0], direction) < 0.0f)
				{
					direction = -direction;
				}

				simplex[3] = support_m(direction);

				if (dot(simplex[3], direction) < 0) // TODO: epsilon
				{
					return false;
				}

				simplex_count = 4;

				break;
			}

			case 4: // pyramid case
			{
				vec3 const& a = simplex[0];
				vec3 const& c = simplex[1];
				vec3 const& b = simplex[2];
				vec3 const& d = simplex[3];

				vec3 da = a - d;
				vec3 db = b - d;
				vec3 dc = c - d;
				vec3 d_o = origin - d;

				// TODO: make sure winding order is currect, don't check dot.
				vec3 dab_perp = cross(da, db);
				if (dot(dab_perp, dc) > 0.0f)
				{
					dab_perp = -dab_perp;
				}

				vec3 dbc_perp = cross(db, dc);
				if (dot(dbc_perp, da) > 0.0f)
				{
					dbc_perp = -dbc_perp;
				}

				vec3 dca_perp = cross(dc, da);
				if (dot(dca_perp, db) > 0.0f)
				{
					dca_perp = -dca_perp;
				}

				real dot_dab = dot(dab_perp, d_o);
				real dot_dbc = dot(dbc_perp, d_o);
				real dot_dca = dot(dca_perp, d_o);

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

	static vec3 project_point_to_triangle(const vec3& point, vec3 const& p1, vec3 const& p2, vec3 const& p3)
	{
		vec3 edge0 = p2 - p1;
		vec3 edge1 = p3 - p1;
		vec3 v0 = p1 - point;

		float a = dot(edge0, edge0);
		float b = dot(edge0, edge1);
		float c = dot(edge1, edge1);
		float d = dot(edge0, v0);
		float e = dot(edge1, v0);

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

	vec3 simplex_normal(array<vec3, 4> const& simplex, s32 index)
	{
		vec3 const& p1 = simplex[index];
		vec3 const& p2 = simplex[(index + 1) % simplex.size()];
		vec3 const& p3 = simplex[(index + 2) % simplex.size()];
		vec3 const& w = simplex[(index + 3) % simplex.size()];
		vec3 n = normalize(cross(p2 - p1, p3 - p2));
		if (dot(n, normalize(w - p1)) > 0.0f)
		{
			n = -n;
		}
		return n;
	}

	struct pptsr
	{
		s32 index;
		vec3 point;
		real distance;
		vec3 normal;
	};

	static pptsr project_point_to_simplex(vec3 const& point, array<vec3, 4> const& simplex)
	{
		// TODO: handle case if point overlapps

		pptsr min = {.distance = INFINITY};

		for (s32 i = 0; i < simplex.size(); i++)
		{
			vec3 const& p1 = simplex[i];
			vec3 const& p2 = simplex[(i + 1) % simplex.size()];
			vec3 const& p3 = simplex[(i + 2) % simplex.size()];

			vec3 proj = project_point_to_triangle(point, p1, p2, p3);
			real dist = distance(point, proj);
			vec3 n = simplex_normal(simplex, i);

			if (dist == min.distance) // may need epsilon
			{
				real d1 = dot(min.normal, normalize(min.point - point));
				real d2 = dot(n, normalize(proj - point));
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

#define NIGHT_DB_DRAW_CSO_RESOLUTION 10
#define NIGHT_GJK_SHAPECAST_ESPILON 0.000001

	ShapeCastResult3D night::gjk::shape_cast_impl(ShapeCastParams3D const& params, u8 skip_t1)
	{
		//DB_ALGO_PUSH("GJK Shapecast"); // TODO: make different aglorithm name for the inverse shapecast

		ShapeCastResult3D result;
		result.t0 = INFINITY;
		result.n0 = vec3(0);
		result.result = false;

		auto support_cso = [DB_ALGO_LAMBDA_CAPTURE](const vec3& direction) -> vec3
			{
				vec3 supa = params.support_casted(direction);
				vec3 supb = params.support_against(-direction);
				return supa - supb;
			};

		vec3 motion = -params.motion;
		vec3 origin = ORIGIN;

		//DB_ALGO_INCREMENT_STEP();
		auto db_fn = [=]()
			{
				// TODO: handle the problem of calling this many times
				uset<vec3> cso_points;

				vec3 const& direction = RIGHT;
				vec3 const& i_axis = FORWARD;
				vec3 const& j_axis = UP;

				for (s32 i = 0; i < NIGHT_DB_DRAW_CSO_RESOLUTION; i++)
				{
					real ti = ((real)i / (real)(NIGHT_DB_DRAW_CSO_RESOLUTION - 1)) * R_PI / 2;
					mat4 rot_i = glm::rotate(ti, i_axis);
					vec3 i_direction = rot_i * vec4(direction, 1);

					for (s32 j = 0; j < NIGHT_DB_DRAW_CSO_RESOLUTION; j++)
					{
						real tj = ((real)j / (real)(NIGHT_DB_DRAW_CSO_RESOLUTION - 1)) * R_PI / 2;
						mat4 rot_j = glm::rotate(tj, j_axis);
						vec3 j_direction = rot_j * vec4(i_direction, 1);

						array<vec3, 8> directions;

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
							vec3 point = support_cso(k);
							cso_points.insert(point);
						}
					}
				}

				for (const auto& i : cso_points)
				{
					DB_ALGO_DRAW_POINT(i, LIGHT_BLUE);
				}
			};
		DB_ALGO_DRAW_POINT(ORIGIN, RED);
		DB_ALGO_DRAW_LINE(ORIGIN, -motion * 1000.0f, RED.opaqued(0.5f));
		DB_ALGO_DRAW_FN(db_fn);
		DB_ALGO_PUSH("Shapecast Iterations");

		array<vec3, 4> simplex = {};
		vec3& a = simplex[0];
		vec3& b = simplex[1];
		vec3& c = simplex[2];
		vec3& w = simplex[3];

		//vec3 search_a = normalize(motion);
		//vec3 search_b = perpendicular_vector(search_a);
		vec3 search_a = perpendicular_vector(motion);
		vec3 search_b = -search_a;
		a = support_cso(search_a);
		b = support_cso(search_b);

		//s32 simplex_size = 2;

		DB_ALGO_INCREMENT_STEP();
		DB_ALGO_DRAW_LINE(a, b, CYAN);

		//u8 x = false;
		for (s32 i = 0; i < params.max_iterations; i++)
		{
			// line case:
			{
				vec3 ab = b - a;
				vec3 n = a - -motion * 1000.0f; // TODO: fix very stupid solution.
				vec3 abdir = cross(ab, n);

				// epsilon error
				real d = dot(abdir, abdir);
				if (d < NIGHT_GJK_SHAPECAST_ESPILON)
				{
					break;
				}

				vec3 abc = cross(abdir, ab);
				vec3 normal = normalize(abc);
				vec3 projected = dot(n, normal) / dot(normal, motion) * motion;
				vec3 projected_normal = cross(ab, projected - n);

				vec3 search_direction;

				real d2 = dot(projected_normal, projected_normal);
				if (d2 < NIGHT_GJK_SHAPECAST_ESPILON)
				{
					real t = dot(projected - n, ab) / dot(ab, ab);
					if (t >= 0 && t <= 1)
					{
						// triangle edge lies near ray, continue as if it was a hit
						search_direction = cross(normal, ab);
						c = support_cso(search_direction);

						auto rc = raycast::plane(origin, motion, a, normal);
						if (rc.result)
						{
							result.t0 = rc.t0;
							result.n0 = rc.n0;
							result.result = true;
						}


						//simplex_size = 3;

						DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);
						DB_ALGO_INCREMENT_STEP();

#ifdef NIGHT_DBAR
						{
							DB_ALGO_DRAW_LINE(a, b, CYAN);
							DB_ALGO_DRAW_LINE(b, c, CYAN);
							DB_ALGO_DRAW_LINE(c, a, CYAN);
							DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);
							if (dot(cross(c - a, b - a), motion) > 0)
							{
								SWAP(a, b);
							}
						}
#endif

						break;
					}
					else
					{
						search_direction = cross(normal, ab);
						c = support_cso(search_direction);
					}
				}
				else
				{
					search_direction = cross(projected_normal, ab);
					c = support_cso(search_direction);
				}

				DB_ALGO_INCREMENT_STEP();
				DB_ALGO_DRAW_LINE(a, b, CYAN);
				DB_ALGO_DRAW_ARROW(ORIGIN, search_direction, PURPLE);

				// correct winding order:
				if (dot(cross(c - a, b - a), motion) > 0)
				{
					SWAP(a, b);
				}

				//simplex_size = 3;
			}

			// triangle case:
			{

				DB_ALGO_INCREMENT_STEP();
				{
					DB_ALGO_DRAW_LINE(a, b, CYAN);
					DB_ALGO_DRAW_LINE(b, c, CYAN);
					DB_ALGO_DRAW_LINE(c, a, CYAN);
					DB_ALGO_DRAW_ARROW((a + b + c) / 3.0f, normalize(cross(b - a, c - a)), ORANGE);
				}

				// epsilon error: the triangle is a thin line.
				vec3 abc = cross(b - a, c - a);
				real d = dot(abc, abc);
				if (d < params.epsilon + NIGHT_GJK_SHAPECAST_ESPILON)
				{
					break;
				}

				// epsilon error: the triangle tangent with the motion, or facing away from the motion
				if (dot(abc, motion) < NIGHT_GJK_SHAPECAST_ESPILON)
				{
					break;
				}

				// TODO: optimize out triangle raycast
				auto rct = raycast::triangle(ORIGIN, motion, a, b, c);

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
			DB_ALGO_POP();
			//DB_ALGO_POP();
			return result;
		}

		// inch tetrahedron towards -motion
		for (s32 i = 0; i < params.max_iterations; i++)
		{
			// triangle case:
			{
				if (dot(cross(c - a, b - a), motion) > 0) // TODO: remove.
				{
					SWAP(a, b);
				}

				vec3 n = normalize(cross(b - a, c - b));

				DB_ALGO_INCREMENT_STEP();
				{
					DB_ALGO_DRAW_ARROW((a + b + c) / 3.0f, n, ORANGE);
					DB_ALGO_DRAW_ARROW(origin, n, PURPLE);
					DB_ALGO_DRAW_LINE(a, b, CYAN);
					DB_ALGO_DRAW_LINE(b, c, CYAN);
					DB_ALGO_DRAW_LINE(c, a, CYAN);
				}

				w = support_cso(n);

				// new support point lies within epsilon distance to the plane of the triangle, 
				// the algorithm is finished. return the triangle's raycast.
				real d = distance_to_plane(w, a, n);
				if (d < params.epsilon)
				{
					//DB_ALGO_POP();
					//DB_ALGO_POP();
					//return result;
					break;
				}

				//simplex_size = 4;
			}

			// tetrahedron case:
			DB_ALGO_INCREMENT_STEP();
			{
				DB_ALGO_DRAW_LINE(a, b, CYAN.opaqued(0.75f));
				DB_ALGO_DRAW_LINE(b, c, CYAN.opaqued(0.75f));
				DB_ALGO_DRAW_LINE(c, a, CYAN.opaqued(0.75f));

				DB_ALGO_DRAW_LINE(a, w, CYAN);
				DB_ALGO_DRAW_LINE(b, w, CYAN);
				DB_ALGO_DRAW_LINE(c, w, CYAN);
			}

			real max_coord = -INFINITY;
			real mc_t = INFINITY;
			vec3 mc_normal = vec3(0);
			vec3 mc_p1 = {};
			vec3 mc_p2 = {};

			for (s32 j = 0; j < 3; j++)
			{
				vec3 const& p1 = simplex[j];
				vec3 const& p2 = simplex[(j + 1) % 3];
				//vec3 w2 = w;

				vec3 n = normalize(cross(p2 - p1, w - p2));
				DB_ALGO_DRAW_ARROW((p1 + p2 + w) / 3.0f, n, ORANGE);

				// skip backfacing triangles.
				real d = dot(motion, n);
				if (d <= NIGHT_GJK_SHAPECAST_ESPILON)
				{
					continue;
				}

				auto rct = raycast::triangle(origin, motion, p1, p2, w);
				real coord = MIN(rct.coordinate.x, rct.coordinate.y);
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

			DB_ALGO_DRAW_POINT(origin + motion * mc_t, RED);

			// correct winding order
			if (dot(cross(c - a, b - a), motion) > 0)
			{
				SWAP(a, b);
			}

			vec3 abc = cross(b - a, c - a);
			if (dot(abc, abc) < NIGHT_GJK_SHAPECAST_ESPILON)
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
	DB_ALGO_DRAW_LINE(a, b, CYAN);
	DB_ALGO_DRAW_LINE(b, c, CYAN);
	DB_ALGO_DRAW_LINE(c, a, CYAN);
	DB_ALGO_POP();
	//DB_ALGO_POP();

	return result;
	}

	ShapeCastResult3D gjk::shape_cast(ShapeCastParams3D const& params, u8 skip_t1)
	{
		ShapeCastResult3D forward;
		ShapeCastResult3D inverse;
		ShapeCastParams3D iparams = params;
		iparams.motion = -iparams.motion;
		DB_ALGO_PUSH("GJK Shapecast");
		forward = shape_cast_impl(iparams, skip_t1);
		DB_ALGO_POP();
		forward.t0 = -forward.t0;

		ShapeCastResult3D combined;
		
		combined.t0 = forward.t0;
		combined.n0 = forward.n0;

		if (!skip_t1 && forward.result)
		{
			DB_ALGO_PUSH("GJK Inverse Shapecast");
			inverse = shape_cast_impl(params, skip_t1);
			DB_ALGO_POP();
			combined.result = (forward.result && inverse.result);
			combined.t1 = inverse.t0;
			combined.n1 = inverse.n0;
		}
		else
		{
			combined.result = forward.result;
			combined.t1 = INFINITY;
			combined.n1 = vec3(0);
		}
		
		return combined;
	}
}