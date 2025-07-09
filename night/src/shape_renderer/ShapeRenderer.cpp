
#include "nightpch.h"
#include "ShapeRenderer.h"
#include "renderer/RenderGraph.h"
#include "texture/ITexture.h"
#include "geometry/Plane.h"
#include "profiler/Profiler.h"

#include "log/log.h"

// TODO: handle texture coords

namespace night
{

	void ShapeRenderer::draw_box(DrawBoxParams const& params)
	{
		ASSERT(false); // TODO: implement
	}

	SphereBackfacePlane ShapeRenderer::sphere_backface_plane(vec3 const& origin, real const& radius, vec3 const& point)
	{
		vec3 const& eye_location = point;
		vec3 oe = normalize(eye_location - origin);

		real inv_dist = radius / distance(eye_location, origin);
		vec3 ellipse_origin = origin + oe * (inv_dist * radius);
		real d = distance(ellipse_origin, origin); // TODO: can probably avoid distance here

		real det = radius * radius - d * d;
		if (det <= 0.0f)
		{
			return { .origin = ellipse_origin, .normal = oe, .radius = 0.0f };
		}

		real ellipse_radius = sqrt(det);

		return { .origin = ellipse_origin, .normal = oe, .radius = ellipse_radius };
	}

	SphereBackfacePlane ShapeRenderer::sphere_backface_plane(vec3 const& origin, real const& radius, Camera const& camera)
	{
		if (camera.type == ECameraType::Orthographic)
		{
			return { .origin = origin, .normal = -camera.direction(), .radius = radius };
		}

		return sphere_backface_plane(origin, radius, camera.translation);
	}

	void ShapeRenderer::draw_sphere(DrawSphereParams const& params)
	{
		NIGHT_PROFILER_SCOPED(SR_draw_sphere);
		ASSERT(params.out_graph != nullptr);

		RenderGraph& out_graph = *params.out_graph;
		auto crt = out_graph.current_render_target();
		ASSERT(crt != nullptr);
		Camera const& camera = crt->camera();

		auto fn = [&](real u, real v) -> Vertex
		{
			Vertex result;
			real r = sin(R_PI * v);
			result.point.x = (r * cos(2.0f * R_PI * u)) * params.radius + params.origin.x;
			result.point.y = (r * sin(2.0f * R_PI * u)) * params.radius + params.origin.y;
			result.point.z = (cos(R_PI * v)) * params.radius + params.origin.z;
			result.point.w = 1.0f;

			result.color = params.color;

			result.texture_coord.x = u;
			result.texture_coord.y = v;
			return result;
		};

		// TODO: add sphere rendering capabilities to renderer, a quad that uses sqrt in fragment shader to calc depth.
		s32 segments = (s32)(24.0f * params.resolution);

		if (!params.outline_only)
		{
			for (s32 i = 0; i < segments; i++)
			{
				for (s32 j = 0; j < segments; j++)
				{
					real u0 = (real)i / (real)segments;
					real u1 = (real)(i + 1) / (real)segments;
					real v0 = (real)j / (real)segments;
					real v1 = (real)(j + 1) / real(segments);

					Triangle tri_1;
					Triangle tri_2;

					tri_1.vertices[0] = fn(u0, v0);
					tri_1.vertices[1] = fn(u1, v0);
					
					Vertex v = fn(u0, v1);
					tri_1.vertices[2] = v;
					tri_2.vertices[0] = v;

					vec3 n = -cross(vec3(tri_1.vertices[1].point) - vec3(tri_1.vertices[0].point), vec3(tri_1.vertices[2].point) - vec3(tri_1.vertices[1].point));
					if (camera.should_cull_plane(vec3(tri_1.vertices[0].point), n))
					{
						continue;
					}

					tri_2.vertices[1] = fn(u1, v0);
					tri_2.vertices[2] = fn(u1, v1);

					if (!params.wireframe)
					{
						out_graph.draw_triangle(tri_1);
						out_graph.draw_triangle(tri_2);
					}
					else if (!params.outline_only)
					{
						out_graph.draw_line(tri_1.vertices[0].point, tri_1.vertices[1].point, params.color, params.width);
						out_graph.draw_line(tri_1.vertices[1].point, tri_1.vertices[2].point, params.color, params.width);
						out_graph.draw_line(tri_1.vertices[2].point, tri_1.vertices[0].point, params.color, params.width);
					}
				}
			}
		}

		if (params.wireframe)
		{
			vec3 const& eye_location = camera.translation;
			vec3 const& origin = params.origin;

			auto [ellipse_origin, ellipse_normal, ellipse_radius] = ShapeRenderer::sphere_backface_plane(origin, params.radius, camera);

			mat4 forward_to_ellipse_normal = rotate_about_vector(FORWARD, ellipse_normal);

			for (s32 i = 0; i < segments; i++)
			{
				real t1 = (real)i / (real)(segments - 1) * R_PI * 2;
				real t2 = (real)(i + 1) / (real)(segments - 1) * R_PI * 2;

				vec3 p1;
				p1.x = cos(t1);
				p1.y = sin(t1);
				p1.z = 0;

				vec3 p2;
				p2.x = cos(t2);
				p2.y = sin(t2);
				p2.z = 0;

				p1 *= ellipse_radius;
				p2 *= ellipse_radius;
				p1 = forward_to_ellipse_normal * vec4(p1, 1);
				p2 = forward_to_ellipse_normal * vec4(p2, 1);
				p1 += ellipse_origin;
				p2 += ellipse_origin;

				out_graph.draw_line(p1, p2, params.color, params.width);
			}
		}
	}

#define BACKFACE_PLANE_EPSILON 0.00001

#ifdef false
	Tube::BackfacePlane Tube::backface_plane(vec3 const& origin, vec3 direction, vec2 radii, real height, Camera const& camera)
	{
		vec3 cap_a_origin = origin + direction * height;
		vec3 cap_b_origin = origin - direction * height;
		vec3 cap_a_normal = direction;
		vec3 cap_b_normal = -direction;
		real const& cap_a_radius = radii.x;
		real const& cap_b_radius = radii.y;

		auto cylinder_case = [&]() -> Tube::BackfacePlane
			{
				Tube::BackfacePlane result;

				vec3 undesired_angle = direction * dot(-camera.direction(), direction);
				vec3 desired_angle = normalize(-camera.direction() - undesired_angle);
				vec3 normal_perp = normalize(cross(desired_angle, direction));

				result.edge_1_a = cap_a_origin + normal_perp * cap_a_radius;
				result.edge_1_b = cap_b_origin + normal_perp * cap_b_radius;
				result.edge_2_a = cap_a_origin - normal_perp * cap_a_radius;
				result.edge_2_b = cap_b_origin - normal_perp * cap_b_radius;
				result.normal = desired_angle;

				return result;
			};

		Tube::BackfacePlane result;

		if (radii.x == radii.y)
		{
			if (camera.type == ECameraType::Orthographic)
			{
				return cylinder_case();
			}

			// TODO: fix this case
			{
				auto backface = Sphere::backface_plane(cap_a_origin, cap_a_radius, camera);
				auto pi = Intersection::infinite_planes(cap_a_origin, direction, backface.origin, backface.normal);
				auto rc = raycast::sphere(pi.origin, pi.direction, cap_a_origin, cap_a_radius);

				if (!rc.result)
				{
					return { .normal = vec3(0) };
				}

				result.edge_1_a = pi.origin + pi.direction * rc.t0; // for some reason we need to subtract here.
				result.edge_2_a = pi.origin + pi.direction * rc.t1;
			}

			{
				auto backface = Sphere::backface_plane(cap_b_origin, cap_b_radius, camera);
				auto pi = Intersection::infinite_planes(cap_b_origin, direction, backface.origin, backface.normal);
				auto rc = raycast::sphere(pi.origin, pi.direction, cap_b_origin, cap_b_radius);

				if (!rc.result)
				{
					return { .normal = vec3(0) };
				}

				result.edge_1_b = pi.origin + pi.direction * rc.t0;
				result.edge_2_b = pi.origin + pi.direction * rc.t1;
			}
			result.normal = -cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

			return result;
		}
		else
		{
			if (camera.type == ECameraType::Orthographic)
			{
				// TODO: use this solution for all cases.
				vec3 camera_direction = -camera.direction();
				auto cot = cone_of_tube(origin, direction, radii, height);

				real d = dot(camera_direction, direction);
				if (abs(d) < TUBE_BACKFACE_PLANE_EPSILON)
				{
					return cylinder_case();
				}
				else
				{
					auto rc = raycast::plane(cot.tip_origin, camera_direction, cot.base_origin, direction);

					vec3 contact = rc.contact(cot.tip_origin, camera_direction);
					auto sbf = Sphere::backface_plane_to_point(cot.base_origin, cot.base_radius, contact);

					if (sbf.radius == 0)
					{
						return { .normal = vec3(0) };
					}

					vec3 perp = normalize(cross(sbf.normal, camera_direction));

					vec3 p1 = sbf.origin + perp * sbf.radius;
					vec3 p2 = sbf.origin - perp * sbf.radius;

					result.edge_1_a = p1;
					result.edge_1_b = cot.tip_origin;
					result.edge_2_a = p2;
					result.edge_2_b = cot.tip_origin;
					result.normal = normalize(cross(p2 - p1, p2 - cot.tip_origin));

					return result;
				}
			}

			// TODO: fix this case.
			auto cot = cone_of_tube(origin, direction, radii, height);

			real proj_t = raycast::project_point_to_ray(camera.translation, cot.tip_origin, cot.base_origin - cot.tip_origin);
			real proj_radius = cot.base_radius * proj_t;
			vec3 proj_origin = cot.tip_origin + (cot.base_origin - cot.tip_origin) * proj_t;

			auto sbf = Sphere::backface_plane(proj_origin, proj_radius, camera);
			auto ip = Intersection::infinite_planes(proj_origin, direction, sbf.origin, sbf.normal);
			auto rc = raycast::sphere(ip.origin, ip.direction, proj_origin, proj_radius);

			if (!rc.result)
			{
				return { .normal = vec3(0) };
			}

			vec3 p1 = ip.origin + ip.direction * rc.t0;
			vec3 p2 = ip.origin + ip.direction * rc.t1;

			auto ca_rc_p1 = raycast::plane(p1, cot.tip_origin - p1, cot.base_origin, direction);
			auto cb_rc_p1 = raycast::plane(p1, cot.tip_origin - p1, cot.base_origin == cap_a_origin ? cap_b_origin : cap_a_origin, direction);
			auto ca_rc_p2 = raycast::plane(p2, cot.tip_origin - p2, cot.base_origin, direction);
			auto cb_rc_p2 = raycast::plane(p2, cot.tip_origin - p2, cot.base_origin == cap_a_origin ? cap_b_origin : cap_a_origin, direction);

			result.edge_1_a = ca_rc_p1.contact(p1, cot.tip_origin - p1);
			result.edge_1_b = cb_rc_p1.contact(p1, cot.tip_origin - p1); // TODO: calculate point of cone convergence, clip to cap_b plane
			result.edge_2_a = ca_rc_p2.contact(p2, cot.tip_origin - p2);
			result.edge_2_b = cb_rc_p2.contact(p2, cot.tip_origin - p2);

			if (proj_radius < 0.0f)
			{
				SWAP(result.edge_1_a, result.edge_2_a);
				SWAP(result.edge_1_b, result.edge_2_b);
			}

			result.normal = -cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

			return result;
		}
	}
#endif

	CylinderBackfacePlane ShapeRenderer::cylinder_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera)
	{
		CylinderBackfacePlane result;
		vec3 cap_a_origin = origin + direction * height;
		vec3 cap_b_origin = origin - direction * height;
		vec3 cap_a_normal = direction;
		vec3 cap_b_normal = -direction;

		if (camera.type == ECameraType::Orthographic)
		{
			vec3 undesired_angle = direction * dot(-camera.direction(), direction);
			vec3 desired_angle = normalize(-camera.direction() - undesired_angle);
			vec3 normal_perp = normalize(cross(desired_angle, direction));

			result.edge_1_a = cap_a_origin + normal_perp * radius;
			result.edge_1_b = cap_b_origin + normal_perp * radius;
			result.edge_2_a = cap_a_origin - normal_perp * radius;
			result.edge_2_b = cap_b_origin - normal_perp * radius;
			result.normal = desired_angle;

			return result;
		}
		else if (camera.type == ECameraType::Perspective)
		{
			//ASSERT(false); // TODO: implement
#ifdef false
			// TODO: fix this case
			{
				auto backface = Sphere::backface_plane(cap_a_origin, cap_a_radius, camera);
				auto pi = Intersection::infinite_planes(cap_a_origin, direction, backface.origin, backface.normal);
				auto rc = raycast::sphere(pi.origin, pi.direction, cap_a_origin, cap_a_radius);

				if (!rc.result)
				{
					return { .normal = vec3(0) };
				}

				result.edge_1_a = pi.origin + pi.direction * rc.t0; // for some reason we need to subtract here.
				result.edge_2_a = pi.origin + pi.direction * rc.t1;
			}

			{
				auto backface = Sphere::backface_plane(cap_b_origin, cap_b_radius, camera);
				auto pi = Intersection::infinite_planes(cap_b_origin, direction, backface.origin, backface.normal);
				auto rc = raycast::sphere(pi.origin, pi.direction, cap_b_origin, cap_b_radius);

				if (!rc.result)
				{
					return { .normal = vec3(0) };
				}

				result.edge_1_b = pi.origin + pi.direction * rc.t0;
				result.edge_2_b = pi.origin + pi.direction * rc.t1;
			}
			result.normal = -cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

			return result;
#endif
		}

		return {};
	}

	ConeBackfacePlane ShapeRenderer::cone_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera)
	{
		ConeBackfacePlane result;

		vec3 base_origin = origin + direction * height;
		vec3 base_normal = direction;

		vec3 tip_origin = origin - direction * height;
		vec3 tip_normal = -direction;

		if (camera.type == ECameraType::Orthographic)
		{
			// TODO: use this solution for all cases.
			vec3 camera_direction = -camera.direction();

			real d = dot(camera_direction, direction);
			if (abs(d) < BACKFACE_PLANE_EPSILON)
			{
				vec3 undesired_angle = direction * dot(-camera.direction(), direction);
				vec3 desired_angle = normalize(-camera.direction() - undesired_angle);
				vec3 normal_perp = normalize(cross(desired_angle, direction));

				result.edge_1_a = base_origin + normal_perp * radius;
				result.edge_1_b = tip_origin;
				result.edge_2_a = base_origin - normal_perp * radius;
				result.edge_2_b = tip_origin;
				result.normal = desired_angle;

				return result;
			}
			else
			{
				auto rc = raycast::plane(tip_origin, camera_direction, base_origin, direction);

				vec3 contact = rc.contact(tip_origin, camera_direction);
				auto sbf = ShapeRenderer::sphere_backface_plane(base_origin, radius, contact);

				if (sbf.radius == 0)
				{
					return { .normal = vec3(0) };
				}

				vec3 perp = normalize(cross(sbf.normal, camera_direction));

				vec3 p1 = sbf.origin + perp * sbf.radius;
				vec3 p2 = sbf.origin - perp * sbf.radius;

				result.edge_1_a = p1;
				result.edge_1_b = tip_origin;
				result.edge_2_a = p2;
				result.edge_2_b = tip_origin;
				result.normal = normalize(cross(p2 - p1, p2 - tip_origin));

				return result;
			}
		}
		else if (camera.type == ECameraType::Orthographic)
		{
			ASSERT(false); // TODO: implement
		}
		
#if 0
		// TODO: fix this case.
		auto cot = cone_of_tube(origin, direction, radii, height);

		real proj_t = raycast::project_point_to_ray(camera.translation, cot.tip_origin, cot.base_origin - cot.tip_origin);
		real proj_radius = cot.base_radius * proj_t;
		vec3 proj_origin = cot.tip_origin + (cot.base_origin - cot.tip_origin) * proj_t;

		auto sbf = Sphere::backface_plane(proj_origin, proj_radius, camera);
		auto ip = Intersection::infinite_planes(proj_origin, direction, sbf.origin, sbf.normal);
		auto rc = raycast::sphere(ip.origin, ip.direction, proj_origin, proj_radius);

		if (!rc.result)
		{
			return { .normal = vec3(0) };
		}

		vec3 p1 = ip.origin + ip.direction * rc.t0;
		vec3 p2 = ip.origin + ip.direction * rc.t1;

		auto ca_rc_p1 = raycast::plane(p1, cot.tip_origin - p1, cot.base_origin, direction);
		auto cb_rc_p1 = raycast::plane(p1, cot.tip_origin - p1, cot.base_origin == cap_a_origin ? cap_b_origin : cap_a_origin, direction);
		auto ca_rc_p2 = raycast::plane(p2, cot.tip_origin - p2, cot.base_origin, direction);
		auto cb_rc_p2 = raycast::plane(p2, cot.tip_origin - p2, cot.base_origin == cap_a_origin ? cap_b_origin : cap_a_origin, direction);

		result.edge_1_a = ca_rc_p1.contact(p1, cot.tip_origin - p1);
		result.edge_1_b = cb_rc_p1.contact(p1, cot.tip_origin - p1); // TODO: calculate point of cone convergence, clip to cap_b plane
		result.edge_2_a = ca_rc_p2.contact(p2, cot.tip_origin - p2);
		result.edge_2_b = cb_rc_p2.contact(p2, cot.tip_origin - p2);

		if (proj_radius < 0.0f)
		{
			SWAP(result.edge_1_a, result.edge_2_a);
			SWAP(result.edge_1_b, result.edge_2_b);
		}

		result.normal = -cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

#endif
		return {};
	}

	static void _draw_tube(DrawCylinderParams const& params, CylinderBackfacePlane const& backface_plane, u8 is_cone)
	{
		NIGHT_PROFILER_SCOPED(SR__draw_tube);
		ASSERT(params.out_graph != nullptr);
		RenderGraph& out_graph = *params.out_graph;
		handle<const ITexture> const& crt = out_graph.current_render_target();
		ASSERT(crt != nullptr);

		s32 segments = s32(128.0f * params.resolution);
		Camera const& camera = crt->camera();
		vec3 camera_direction = camera.direction();

		// TODO: fix gap when culling cap backface.
		vec3 cap_a = params.origin + params.direction * params.height;
		vec3 cap_b = params.origin - params.direction * params.height;

		u8 cull_cap_a = camera.should_cull_plane(cap_a, params.direction);
		u8 cull_cap_b = camera.should_cull_plane(cap_b, -params.direction);

		//auto backface_plane = cylinder_backface_plane(params.origin, params.direction, params.radius, params.height, camera);

		mat4 ftd;
		ftd = rotate_about_vector(FORWARD, params.direction);

		constexpr real epsilon = 0.000001f;
		real d = dot(camera_direction, params.direction);
		u8 x = abs(d) < 1.0f - epsilon;

		if (x)
		{
			if (params.wireframe)
			{
				{
					DrawLineParams dlp;
					dlp.p1 = backface_plane.edge_1_a;
					dlp.p2 = backface_plane.edge_1_b;
					dlp.color = params.color;
					dlp.color2 = params.color;
					dlp.width = params.width;
					dlp.width2 = params.width;

					out_graph.draw_line(dlp);
				}

				{
					DrawLineParams dlp;
					dlp.p1 = backface_plane.edge_2_a;
					dlp.p2 = backface_plane.edge_2_b;
					dlp.color = params.color;
					dlp.color2 = params.color;
					dlp.width = params.width;
					dlp.width2 = params.width;

					out_graph.draw_line(dlp);
				}
			}
		}

		if (!params.wireframe)
		{
			real cap_b_radius = is_cone ? 0.0f : params.radius;
			for (s32 i = 0; i < segments; i++)
			{
				real t1 = ((real)i / (real)(segments)) * R_PI * 2;

				real t2 = ((real)(i + 1) / (real)(segments)) * R_PI * 2;

				vec3 p1;
				p1.x = sin(t1);
				p1.y = cos(t1);
				p1.z = 0;

				vec3 p2;
				p2.x = sin(t2);
				p2.y = cos(t2);
				p2.z = 0;

				p1 = ftd * vec4(p1, 1);
				p2 = ftd * vec4(p2, 1);

				vec3 ca1 = p1 * params.radius + params.origin + params.direction * params.height;
				vec3 cb1 = p1 * cap_b_radius + params.origin - params.direction * params.height;
				vec3 ca2 = p2 * params.radius + params.origin + params.direction * params.height;
				vec3 cb2 = p2 * cap_b_radius + params.origin - params.direction * params.height;

				if (x)
				{
					vec3 n = cross(ca2 - ca1, cb2 - ca2);
					if (!camera.should_cull_plane(ca1, n))
					{
						Triangle tri1;
						tri1.vertices[0].point = vec4(ca1, 1);
						tri1.vertices[1].point = vec4(cb1, 1);
						tri1.vertices[2].point = vec4(cb2, 1);
						tri1.vertices[0].color = params.color;
						tri1.vertices[1].color = params.color;
						tri1.vertices[2].color = params.color;

						Triangle tri2;
						tri2.vertices[0].point = vec4(cb2, 1);
						tri2.vertices[1].point = vec4(ca2, 1);
						tri2.vertices[2].point = vec4(ca1, 1);
						tri2.vertices[0].color = params.color;
						tri2.vertices[1].color = params.color;
						tri2.vertices[2].color = params.color;

						out_graph.draw_triangle(tri1);
						out_graph.draw_triangle(tri2);
					}
				}

				if (!cull_cap_a)
				{
					Triangle cap_a_tri;
					vec3 ca0 = params.origin + params.direction * params.height;
					cap_a_tri.vertices[0].point = vec4(ca0, 1);
					cap_a_tri.vertices[1].point = vec4(ca1, 1);
					cap_a_tri.vertices[2].point = vec4(ca2, 1);
					cap_a_tri.vertices[0].color = params.color;
					cap_a_tri.vertices[1].color = params.color;
					cap_a_tri.vertices[2].color = params.color;
					out_graph.draw_triangle(cap_a_tri);
				}

				if (!cull_cap_b && !is_cone)
				{
					vec3 cb0 = params.origin - params.direction * params.height;
					Triangle cap_b_tri;
					cap_b_tri.vertices[0].point = vec4(cb0, 1);
					cap_b_tri.vertices[1].point = vec4(cb1, 1);
					cap_b_tri.vertices[2].point = vec4(cb2, 1);
					cap_b_tri.vertices[0].color = params.color;
					cap_b_tri.vertices[1].color = params.color;
					cap_b_tri.vertices[2].color = params.color;
					out_graph.draw_triangle(cap_b_tri);
				}
			}
			return;
		}


		vec3 up_on_cap_plane = ftd * vec4(UP, 1);
		mat4 inv_ftd = glm::inverse(ftd);
		vec3 inv_pn = inv_ftd * vec4(backface_plane.normal, 1);

		auto draw_cap_ff = [&](vec3 const& cap_origin, real const& cap_radius, vec3 const& cap_normal)
			{
				if (params.wireframe)
				{
					for (s32 i = 0; i < segments; i++)
					{
						{
							real t1 = ((real)i / (real)(segments)) * R_PI * 2;

							real t2 = ((real)(i + 1) / (real)(segments)) * R_PI * 2;

							vec3 p1;
							p1.x = sin(t1) * cap_radius;
							p1.y = cos(t1) * cap_radius;
							p1.z = 0;

							vec3 p2;
							p2.x = sin(t2) * cap_radius;
							p2.y = cos(t2) * cap_radius;
							p2.z = 0;

							p1 = ftd * vec4(p1, 1);
							p2 = ftd * vec4(p2, 1);
							p1 += cap_origin;
							p2 += cap_origin;

							DrawLineParams dlp;
							dlp.p1 = p1;
							dlp.p2 = p2;
							dlp.color = params.color;
							dlp.color2 = params.color;
							dlp.width = params.width;
							dlp.width2 = params.width;

							out_graph.draw_line(dlp);
						}
					}
				}
				else
				{
					// TODO: fill out cap convex with triangles
				}
			};

		auto draw_cap_bf = [&](vec3 const& cap_origin, real const& cap_radius, vec3 const& cap_normal)
			{
				ASSERT(params.wireframe); // this function is only for wireframes
				if (params.draw_through_opacity != 0)
				{
					// calculate range of theta we will be drawing the cap in:
					auto ip = line_of_intersection_between_to_planes(cap_origin, -cap_normal, backface_plane.edge_1_a, backface_plane.normal);
					auto sc = raycast::sphere(ip.origin, ip.direction, cap_origin, cap_radius);

					real bmin_theta;
					real bmax_theta;

					if (sc.result)
					{
						vec3 c1 = sc.contact<0>(ip.origin, ip.direction);
						vec3 c2 = sc.contact<1>(ip.origin, ip.direction);
						c1 -= cap_origin;
						c2 -= cap_origin;

						c1 = inv_ftd * vec4(c1, 1);
						c2 = inv_ftd * vec4(c2, 1);

						if (orientation(c1, c2, inv_pn) == EOrientation::CounterClockwise)
						{
							SWAP(c1, c2);
						}

						bmin_theta = angle_clockwise(c1, vec2(UP));
						bmax_theta = bmin_theta + angle_clockwise(c2, c1);

					}
					else
					{
						bmin_theta = 0;
						bmax_theta = R_PI * 2;
					}

					real fmin_theta = bmax_theta;
					real fmax_theta = bmin_theta + R_PI * 2;

					// draw the cap:
					for (s32 i = 0; i < segments; i++)
					{
						// backface:
						{
							real t1 = (real)i / (real)(segments);
							t1 = lerp(bmin_theta, bmax_theta, t1);

							real t2 = (real)(i + 1) / (real)(segments);
							t2 = lerp(bmin_theta, bmax_theta, t2);

							vec3 p1;
							p1.x = sin(t1) * cap_radius;
							p1.y = cos(t1) * cap_radius;
							p1.z = 0;

							vec3 p2;
							p2.x = sin(t2) * cap_radius;
							p2.y = cos(t2) * cap_radius;
							p2.z = 0;

							p1 = ftd * vec4(p1, 1);
							p2 = ftd * vec4(p2, 1);
							p1 += cap_origin;
							p2 += cap_origin;

							DrawLineParams dlp;
							dlp.p1 = p1;
							dlp.p2 = p2;
							dlp.color = params.color.opaqued(params.draw_through_opacity);
							dlp.color2 = params.color.opaqued(params.draw_through_opacity);
							dlp.width = params.width;
							dlp.width2 = params.width;

							out_graph.draw_line(dlp);
						}

						// front face:
						{
							real t1 = (real)i / (real)(segments);
							t1 = lerp(fmin_theta, fmax_theta, t1);

							real t2 = (real)(i + 1) / (real)(segments);
							t2 = lerp(fmin_theta, fmax_theta, t2);

							vec3 p1;
							p1.x = sin(t1) * cap_radius;
							p1.y = cos(t1) * cap_radius;
							p1.z = 0;

							vec3 p2;
							p2.x = sin(t2) * cap_radius;
							p2.y = cos(t2) * cap_radius;
							p2.z = 0;

							p1 = ftd * vec4(p1, 1);
							p2 = ftd * vec4(p2, 1);
							p1 += cap_origin;
							p2 += cap_origin;

							DrawLineParams dlp;
							dlp.p1 = p1;
							dlp.p2 = p2;
							dlp.color = params.color;
							dlp.color2 = params.color;
							dlp.width = params.width;
							dlp.width2 = params.width;

							out_graph.draw_line(dlp);
						}
					}
				}
			};

		//if (params.wireframe)
		{
			if (backface_plane.normal == ORIGIN)
			{
				draw_cap_ff(cap_a, params.radius, params.direction);
				if (!is_cone)
				{
					draw_cap_ff(cap_b, params.radius, params.direction); // TODO: may not want to do this.
				}
			}
			else
			{
				if (cull_cap_a)
				{
					draw_cap_bf(cap_a, params.radius, params.direction);
				}
				else
				{
					draw_cap_ff(cap_a, params.radius, params.direction);
				}

				if (!is_cone)
				{
					if (cull_cap_b)
					{
						draw_cap_bf(cap_b, params.radius, params.direction);
					}
					else
					{
						draw_cap_ff(cap_b, params.radius, params.direction);
					}
				}
			}
		}
	}

	void ShapeRenderer::draw_cylinder(DrawCylinderParams const& params)
	{
		ASSERT(params.out_graph != nullptr);
		handle<const ITexture> const& crt = params.out_graph->current_render_target();
		ASSERT(crt != nullptr);
		Camera const& camera = crt->camera();

		auto backface_plane = ShapeRenderer::cylinder_backface_plane(params.origin, params.direction, params.radius, params.height, camera);
		_draw_tube(params, backface_plane, false);
	}

	void ShapeRenderer::draw_cone(DrawConeParams const& params)
	{
		ASSERT(params.out_graph != nullptr);
		handle<const ITexture> const& crt = params.out_graph->current_render_target();
		ASSERT(crt != nullptr);
		Camera const& camera = crt->camera();

		auto backface_plane = ShapeRenderer::cone_backface_plane(params.origin, params.direction, params.radius, params.height, camera);
		_draw_tube(params, backface_plane, true);
	}

	void ShapeRenderer::draw_convex(DrawConvexParams const& params)
	{
		NIGHT_PROFILER_SCOPED(SR_draw_convex);
		ASSERT(params.planes != nullptr);
		vector<Plane> planes = *params.planes;

		ASSERT(params.out_graph != nullptr); // TODO: draw to renderer if nulltr
		RenderGraph& out_graph = *params.out_graph;
		auto crt = out_graph.current_render_target();
		ASSERT(crt != nullptr);
		vec3 camera_direction = crt->camera().direction();

		auto decomp = decompose(params.transform);

		for (s32 i = 0; i < planes.size(); i++)
		{
			Plane& plane = planes[i];

			plane.normal = Camera::triangle_normal(plane.vertices[0], plane.vertices[1], plane.vertices[2]);
			plane.normal *= decomp.scale;
			plane.normal = decomp.rotation * plane.normal;
			plane.normal = normalize(plane.normal);

			// apply transform:
			for (s32 j = 0; j < plane.vertices.size(); j++)
			{
				vec3& vertex = plane.vertices[j];
				vertex = params.transform * vec4(vertex, 1);
			}

			//plane.normal = Camera::triangle_normal(plane.vertices[0], plane.vertices[1], plane.vertices[2]);

			Color color = params.color;

			// cull backface:
			u8 should_cull = crt->camera().should_cull_plane(plane.vertices[0], plane.normal);
			if (should_cull)
			{
				if (!params.wireframe || params.draw_through_opacity == 0)
				{
					continue;
				}

				color.a *= params.draw_through_opacity;
			}

			// draw convex:
			if (params.wireframe)
			{
				for (s32 j = 0; j < plane.vertices.size(); j++)
				{
					vec3  p1 = plane.vertices[j];
					vec3  p2 = plane.vertices[(j + 1) % plane.vertices.size()];

					if (should_cull)
					{
						p1 += camera_direction * 0.001f;
						p2 += camera_direction * 0.001f;
					}

					DrawLineParams dlp;
					dlp.p1 = p1;
					dlp.p2 = p2;
					dlp.color = color;
					dlp.color2 = color;
					dlp.width = params.width;
					dlp.width2 = params.width;

					out_graph.draw_line(dlp);
				}
			}
			else
			{
				ASSERT(plane.vertices.size() >= 3); // every plane must be at least a triangle

				vec3 const& p0 = plane.vertices[0];

				for (s32 j = 1; j < plane.vertices.size() - 1; j++)
				{
					vec3 const& p1 = plane.vertices[j];
					vec3 const& p2 = plane.vertices[j + 1];

					Triangle triangle;
					triangle.vertices[0].point = vec4(p0, 1);
					triangle.vertices[1].point = vec4(p1, 1);
					triangle.vertices[2].point = vec4(p2, 1);

					triangle.vertices[0].color = params.color;
					triangle.vertices[1].color = params.color;
					triangle.vertices[2].color = params.color;
					out_graph.draw_triangle(triangle);
				}
			}
		}
	}

}