
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



	SphereBackfacePlane ShapeRenderer3D::sphere_backface_plane(vec3 const& origin, real const& radius, vec3 const& point)
	{
		vec3 const& eye_location = point;
		vec3 oe = math::normalize(eye_location - origin);

		real inv_dist = radius / math::length(eye_location - origin);
		vec3 ellipse_origin = origin + oe * (inv_dist * radius);
		real d = math::length(ellipse_origin - origin); // TODO: can probably avoid distance here

		real det = radius * radius - d * d;
		if (det <= 0.0f)
		{
			return { .origin = ellipse_origin, .normal = oe, .radius = 0.0f };
		}

		real ellipse_radius = sqrt(det);

		return { .origin = ellipse_origin, .normal = oe, .radius = ellipse_radius };
	}

	SphereBackfacePlane ShapeRenderer3D::sphere_backface_plane(vec3 const& origin, real const& radius, Camera const& camera)
	{
		if (camera.type == ECameraType::Orthographic)
		{
			return { .origin = origin, .normal = -camera.direction(), .radius = radius };
		}

		return sphere_backface_plane(origin, radius, camera.translation);
	}

#if 0
	void ShapeRenderer3D::draw_sphere(DrawSphereParams const& params)
	{
		NIGHT_PROFILER_SCOPED("ShapeRenderer3D::draw_sphere");
		ASSERT(params.out_graph != nullptr);

		RenderGraph& out_graph = *params.out_graph;
		auto crt = out_graph.current_render_target();
		ASSERT(crt != nullptr);
		Camera const& camera = crt->camera();

		auto fn = [&](real u, real v) -> Vertex<>
		{
			Vertex<> result;
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
					
					Vertex<> v = fn(u0, v1);
					tri_1.vertices[2] = v;
					tri_2.vertices[0] = v;

					vec3 n = -math::cross(vec3(tri_1.vertices[1].point) - vec3(tri_1.vertices[0].point), vec3(tri_1.vertices[2].point) - vec3(tri_1.vertices[1].point));
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

			auto [ellipse_origin, ellipse_normal, ellipse_radius] = ShapeRenderer3D::sphere_backface_plane(origin, params.radius, camera);

			mat4 forward_to_ellipse_normal = math::rotate_about_vector(FORWARD, ellipse_normal);

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
#endif
#define BACKFACE_PLANE_EPSILON NIGHT_EPSILON_MEDIUM

#if 0
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

				vec3 undesired_angle = direction * math::dot(-camera.direction(), direction);
				vec3 desired_angle = math::normalize(-camera.direction() - undesired_angle);
				vec3 normal_perp = math::normalize(math::cross(desired_angle, direction));

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
			result.normal = -math::cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

			return result;
		}
		else
		{
			if (camera.type == ECameraType::Orthographic)
			{
				// TODO: use this solution for all cases.
				vec3 camera_direction = -camera.direction();
				auto cot = cone_of_tube(origin, direction, radii, height);

				real d = math::dot(camera_direction, direction);
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

					vec3 perp = math::normalize(math::cross(sbf.normal, camera_direction));

					vec3 p1 = sbf.origin + perp * sbf.radius;
					vec3 p2 = sbf.origin - perp * sbf.radius;

					result.edge_1_a = p1;
					result.edge_1_b = cot.tip_origin;
					result.edge_2_a = p2;
					result.edge_2_b = cot.tip_origin;
					result.normal = math::normalize(math::cross(p2 - p1, p2 - cot.tip_origin));

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

			result.normal = -math::cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

			return result;
		}
	}
#endif

#if 0
	ConeBackfacePlane ShapeRenderer3D::cone_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera)
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

			real d = math::dot(camera_direction, direction);
			if (abs(d) < BACKFACE_PLANE_EPSILON)
			{
				vec3 undesired_angle = direction * math::dot(-camera.direction(), direction);
				vec3 desired_angle = math::normalize(-camera.direction() - undesired_angle);
				vec3 normal_perp = math::normalize(math::cross(desired_angle, direction));

				result.edge_1_a = base_origin + normal_perp * radius;
				result.edge_1_b = tip_origin;
				result.edge_2_a = base_origin - normal_perp * radius;
				result.edge_2_b = tip_origin;
				result.normal = desired_angle;

				return result;
			}
			else
			{
				auto rc = Raycast3D<>::plane(tip_origin, camera_direction, base_origin, direction);

				vec3 contact = rc.contact(tip_origin, camera_direction);
				auto sbf = ShapeRenderer3D::sphere_backface_plane(base_origin, radius, contact);

				if (sbf.radius == 0 || std::isnan(sbf.radius))
				{
					return { .normal = vec3(0) };
				}

				vec3 perp = math::normalize(math::cross(sbf.normal, camera_direction));

				vec3 p1 = sbf.origin + perp * sbf.radius;
				vec3 p2 = sbf.origin - perp * sbf.radius;

				result.edge_1_a = p1;
				result.edge_1_b = tip_origin;
				result.edge_2_a = p2;
				result.edge_2_b = tip_origin;
				result.normal = math::normalize(math::cross(p2 - p1, p2 - tip_origin));

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

		result.normal = -math::cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

#endif
		return {};
	}

	static void _draw_tube(DrawCylinderParams const& params, CylinderBackfacePlane const& backface_plane, u8 is_cone)
	{
		NIGHT_PROFILER_SCOPED("ShapeRenderer3D::_draw_tube");
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
		ftd = math::rotate_about_vector(FORWARD, params.direction);

		constexpr real epsilon = 0.000001f;
		real d = math::dot(camera_direction, params.direction);
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
					vec3 n = math::cross(ca2 - ca1, cb2 - ca2);
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
		mat4 inv_ftd = math::inverse(ftd);
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
					auto ip = math::line_of_intersection_between_to_planes(cap_origin, -cap_normal, backface_plane.edge_1_a, backface_plane.normal);
					auto sc = Raycast3D<>::sphere(ip.origin, ip.direction, cap_origin, cap_radius);

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

						if (math::orientation(c1, c2, inv_pn) == EOrientation::CounterClockwise)
						{
							SWAP(c1, c2);
						}

						bmin_theta = math::angle_clockwise(c1, vec2(UP));
						bmax_theta = bmin_theta + math::angle_clockwise(c2, c1);

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
							t1 = math::lerp(bmin_theta, bmax_theta, t1);

							real t2 = (real)(i + 1) / (real)(segments);
							t2 = math::lerp(bmin_theta, bmax_theta, t2);

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
							t1 = math::lerp(fmin_theta, fmax_theta, t1);

							real t2 = (real)(i + 1) / (real)(segments);
							t2 = math::lerp(fmin_theta, fmax_theta, t2);

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

	void ShapeRenderer3D::draw_cylinder(DrawCylinderParams const& params)
	{
		ASSERT(params.out_graph != nullptr);
		handle<const ITexture> const& crt = params.out_graph->current_render_target();
		ASSERT(crt != nullptr);
		Camera const& camera = crt->camera();

		auto backface_plane = ShapeRenderer3D::cylinder_backface_plane(params.origin, params.direction, params.radius, params.height, camera);
		_draw_tube(params, backface_plane, false);
	}

	void ShapeRenderer3D::draw_cone(DrawConeParams const& params)
	{
		ASSERT(params.out_graph != nullptr);
		handle<const ITexture> const& crt = params.out_graph->current_render_target();
		ASSERT(crt != nullptr);
		Camera const& camera = crt->camera();

		auto backface_plane = ShapeRenderer3D::cone_backface_plane(params.origin, params.direction, params.radius, params.height, camera);
		_draw_tube(params, backface_plane, true);
	}
#endif

	void ShapeRenderer3D::draw_box(RenderTarget render_target, DrawBoxParams const& params)
	{
		ASSERT(render_target != nullptr);
		DecomposedTransform<> decomp = math::decompose(params.transform);

		constexpr s32 idx_front = 0;
		constexpr s32 idx_back = 1;
		constexpr s32 idx_left = 2;
		constexpr s32 idx_right = 3;
		constexpr s32 idx_top = 4;
		constexpr s32 idx_bottom = 5;

		Camera const& camera = render_target->camera();

		constexpr array<vec3, 6> ufaces =
		{
			FORWARD,
			BACKWARDS,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

		array<vec3, 6> faces;
		faces[idx_front] = math::normalize(decomp.rotation * (ufaces[idx_front] * params.extents * decomp.scale));
		faces[idx_back] = math::normalize(decomp.rotation * (ufaces[idx_back] * params.extents * decomp.scale));
		faces[idx_left] = math::normalize(decomp.rotation * (ufaces[idx_left] * params.extents * decomp.scale));
		faces[idx_right] = math::normalize(decomp.rotation * (ufaces[idx_right] * params.extents * decomp.scale));
		faces[idx_top] = math::normalize(decomp.rotation * (ufaces[idx_top] * params.extents * decomp.scale));
		faces[idx_bottom] = math::normalize(decomp.rotation * (ufaces[idx_bottom] * params.extents * decomp.scale));

		if (params.on_draw_line != nullptr)
		{
			auto fn = [&](s32 f1, s32 f2)
				{
					DrawLineParams dlp;

					// TODO: this can be computed in compile time
					vec3 cross = math::normalize(math::cross(ufaces[f1], ufaces[f2]));

					dlp.p1 = params.transform * vec4((ufaces[f1] + ufaces[f2] + cross) * params.extents, 1);
					dlp.p2 = params.transform * vec4((ufaces[f1] + ufaces[f2] - cross) * params.extents, 1);
					dlp.width = params.width;
					dlp.color = params.color;

					u8 x1 = camera.should_cull_plane(dlp.p1, faces[f2]);
					u8 x2 = camera.should_cull_plane(dlp.p1, faces[f1]);

					if (x1 == false && x2 == false)
					{
						dlp.color = dlp.color.opaqued(params.near_corner_opacity);
					}
					else if (x1 == true && x2 == true)
					{
						dlp.color = dlp.color.opaqued(params.far_corner_opacity);
					}

					params.on_draw_line({ .params = dlp });
				};

			fn(idx_front, idx_top);
			fn(idx_front, idx_right);
			fn(idx_front, idx_bottom);
			fn(idx_front, idx_left);

			fn(idx_back, idx_top);
			fn(idx_back, idx_right);
			fn(idx_back, idx_bottom);
			fn(idx_back, idx_left);

			fn(idx_top, idx_left);
			fn(idx_top, idx_right);
			fn(idx_bottom, idx_left);
			fn(idx_bottom, idx_right);
		}

		if (params.on_draw_triangle != nullptr)
		{
			auto fn = [&](s32 face, s32 up, s32 right)
				{
					vec3 p1 = params.transform * vec4((ufaces[face] + ufaces[up] - ufaces[right]) * params.extents, 1);

					if (camera.should_cull_plane(p1, faces[face]))
					{
						return;
					}

					vec3 p2 = params.transform * vec4((ufaces[face] + ufaces[up] + ufaces[right]) * params.extents, 1);
					vec3 p3 = params.transform * vec4((ufaces[face] - ufaces[up] + ufaces[right]) * params.extents, 1);
					vec3 p4 = params.transform * vec4((ufaces[face] - ufaces[up] - ufaces[right]) * params.extents, 1);

					Triangle t1;
					t1.vertices[0].point = vec4(p1, 1);
					t1.vertices[1].point = vec4(p2, 1);
					t1.vertices[2].point = vec4(p3, 1);
					t1.vertices[0].color = params.color;
					t1.vertices[1].color = params.color;
					t1.vertices[2].color = params.color;

					params.on_draw_triangle({.triangle = t1, .normal = faces[face] });

					Triangle t2;
					t2.vertices[0].point = vec4(p3, 1);
					t2.vertices[1].point = vec4(p4, 1);
					t2.vertices[2].point = vec4(p1, 1);
					t2.vertices[0].color = params.color;
					t2.vertices[1].color = params.color;
					t2.vertices[2].color = params.color;

					params.on_draw_triangle({ .triangle = t2, .normal = faces[face] });
				};

			fn(idx_front, idx_top, idx_right);
			fn(idx_back, idx_top, idx_left);

			fn(idx_top, idx_back, idx_right);
			fn(idx_right, idx_back, idx_bottom);
			fn(idx_bottom, idx_back, idx_left);
			fn(idx_left, idx_back, idx_top);
		};
	}

	void ShapeRenderer3D::draw_pyramid(RenderTarget render_target, DrawPyramidParams const& params)
	{
		ASSERT(render_target != nullptr);
		DecomposedTransform<> decomp = math::decompose(params.transform);

		constexpr s32 idx_back = 0;
		constexpr s32 idx_left = 1;
		constexpr s32 idx_right = 2;
		constexpr s32 idx_top = 3;
		constexpr s32 idx_bottom = 4;

		Camera const& camera = render_target->camera();

		constexpr array<vec3, 6> ufaces =
		{
			BACKWARDS,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

		array<vec3, 6> faces;
		faces[idx_back] = math::normalize(decomp.rotation * (ufaces[idx_back] * params.extents * decomp.scale));
		faces[idx_left] = math::normalize(decomp.rotation * (math::normalize(ufaces[idx_left] + FORWARD / 2.0f) * params.extents * decomp.scale));
		faces[idx_right] = math::normalize(decomp.rotation * (math::normalize(ufaces[idx_right] + FORWARD / 2.0f) * params.extents * decomp.scale));
		faces[idx_top] = math::normalize(decomp.rotation * (math::normalize(ufaces[idx_top] + FORWARD / 2.0f) * params.extents * decomp.scale));
		faces[idx_bottom] = math::normalize(decomp.rotation * (math::normalize(ufaces[idx_bottom] + FORWARD / 2.0f) * params.extents * decomp.scale));

		if (params.on_draw_line != nullptr)
		{
			auto fn = [&](s32 f1, s32 f2, u8 tip)
				{
					DrawLineParams dlp;

					// TODO: this can be computed in compile time
					vec3 cross = math::normalize(math::cross(ufaces[f1], ufaces[f2]));

					dlp.p1 = params.transform * vec4((ufaces[f1] + ufaces[f2] + cross) * params.extents, 1);

					if (!tip)
					{
						dlp.p2 = params.transform * vec4((ufaces[f1] + ufaces[f2] - cross) * params.extents, 1);
					}
					else
					{
						dlp.p2 = params.transform * vec4(FORWARD * params.extents, 1);
					}

					dlp.width = params.width;
					dlp.color = params.color;

					u8 x1 = camera.should_cull_plane(dlp.p1, faces[f2]);
					u8 x2 = camera.should_cull_plane(dlp.p1, faces[f1]);

					if (x1 == false && x2 == false)
					{
						dlp.color = dlp.color.opaqued(params.near_corner_opacity);
					}
					else if (x1 == true && x2 == true)
					{
						dlp.color = dlp.color.opaqued(params.far_corner_opacity);
					}

					params.on_draw_line({ .params = dlp });
				};

			fn(idx_back, idx_top, false);
			fn(idx_back, idx_right, false);
			fn(idx_back, idx_bottom, false);
			fn(idx_back, idx_left, false);

			fn(idx_left, idx_top, true);
			fn(idx_top, idx_right, true);
			fn(idx_bottom, idx_left, true);
			fn(idx_right, idx_bottom, true);
		}

		if (params.on_draw_triangle)
		{
			{
				// base:
				vec3 p1 = params.transform * vec4((ufaces[idx_back] + ufaces[idx_top] - ufaces[idx_left]) * params.extents, 1);

				if (!camera.should_cull_plane(p1, faces[idx_back]))
				{
					vec3 p2 = params.transform * vec4((ufaces[idx_back] + ufaces[idx_top] + ufaces[idx_left]) * params.extents, 1);
					vec3 p3 = params.transform * vec4((ufaces[idx_back] - ufaces[idx_top] + ufaces[idx_left]) * params.extents, 1);
					vec3 p4 = params.transform * vec4((ufaces[idx_back] - ufaces[idx_top] - ufaces[idx_left]) * params.extents, 1);

					Triangle t1;
					t1.vertices[0].point = vec4(p1, 1);
					t1.vertices[1].point = vec4(p2, 1);
					t1.vertices[2].point = vec4(p3, 1);
					t1.vertices[0].color = params.color;
					t1.vertices[1].color = params.color;
					t1.vertices[2].color = params.color;

					params.on_draw_triangle({ .triangle = t1, .normal = faces[idx_back] });

					Triangle t2;
					t2.vertices[0].point = vec4(p3, 1);
					t2.vertices[1].point = vec4(p4, 1);
					t2.vertices[2].point = vec4(p1, 1);
					t2.vertices[0].color = params.color;
					t2.vertices[1].color = params.color;
					t2.vertices[2].color = params.color;

					params.on_draw_triangle({ .triangle = t2, .normal = faces[idx_back] });
				}
			}

			auto fn = [&](s32 face, vec3 const& bl, vec3 const& br)
				{
					vec3 p1 = params.transform * vec4(bl * params.extents, 1);

					if (camera.should_cull_plane(p1, faces[face]))
					{
						return;
					}

					Triangle tri;
					tri.vertices[0].point = vec4(p1, 1);
					tri.vertices[1].point = params.transform * vec4(FORWARD * params.extents, 1);
					tri.vertices[2].point = params.transform * vec4(br * params.extents, 1);
					tri.vertices[0].color = params.color;
					tri.vertices[1].color = params.color;
					tri.vertices[2].color = params.color;

					params.on_draw_triangle({ .triangle = tri, .normal = faces[face] });
				};

			fn(idx_left, vec3(-1.0f, 1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f));
			fn(idx_top, vec3(1.0f, 1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f));
			fn(idx_right, vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, 1.0f, -1.0f));
			fn(idx_bottom, vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f));
		}
	}

	static ConeBackfacePlane _cone_backface_plane2(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera)
	{
		ConeBackfacePlane result;

		vec3 base_origin = origin - direction * height;
		vec3 base_normal = -direction;

		vec3 tip_origin = origin + direction * height;
		vec3 tip_normal = direction;

		if (camera.type == ECameraType::Orthographic)
		{
			// TODO: use this solution for all cases.
			vec3 camera_direction = -camera.direction();

			real d = math::dot(camera_direction, -direction);
			if (abs(d) < BACKFACE_PLANE_EPSILON)
			{
				vec3 undesired_angle = -direction * math::dot(-camera.direction(), -direction);
				vec3 desired_angle = math::normalize(-camera.direction() - undesired_angle);
				vec3 normal_perp = math::normalize(math::cross(desired_angle, -direction));

				result.edge_1_a = base_origin + normal_perp * radius;
				result.edge_1_b = tip_origin;
				result.edge_2_a = base_origin - normal_perp * radius;
				result.edge_2_b = tip_origin;
				result.normal = desired_angle;

				return result;
			}
			else
			{
				auto rc = Raycast3D<>::plane(tip_origin, camera_direction, base_origin, -direction);

				vec3 contact = rc.contact(tip_origin, camera_direction);
				auto sbf = ShapeRenderer3D::sphere_backface_plane(base_origin, radius, contact);

				if (sbf.radius == 0 || std::isnan(sbf.radius))
				{
					return { .normal = vec3(0) };
				}

				vec3 perp = math::normalize(math::cross(sbf.normal, camera_direction));

				vec3 p1 = sbf.origin + perp * sbf.radius;
				vec3 p2 = sbf.origin - perp * sbf.radius;

				result.edge_1_a = p1;
				result.edge_1_b = tip_origin;
				result.edge_2_a = p2;
				result.edge_2_b = tip_origin;
				result.normal = math::normalize(math::cross(p2 - p1, p2 - tip_origin));

				return result;
			}
		}
		else if (camera.type == ECameraType::Orthographic)
		{
			ASSERT(false); // TODO: implement
		}

		return {};
	}

	inline static void _draw_tube2(RenderTarget render_target, vec3 const& origin, vec3 const& direction, DrawCylinderParams2 const& dcp, CylinderBackfacePlane const& backface_plane, u8 is_cone)
	{
		ASSERT(render_target != nullptr);
		Camera const& camera = render_target->camera();
		vec3 camera_direction = camera.direction();
		s32 segments = s32(32.0f * dcp.resolution);

		vec3 cap_a = origin - direction * dcp.height;
		vec3 cap_b = origin + direction * dcp.height;

		u8 cull_cap_a = camera.should_cull_plane(cap_a, -direction);
		u8 cull_cap_b = camera.should_cull_plane(cap_b, direction);

		//vec3 base = origin - direction * dcp.height;
		//
		//u8 cull_base = camera.should_cull_plane(base, -direction);

		mat4 ftd;
		ftd = math::rotate_about_vector(FORWARD, direction);

		real d = math::dot(camera_direction, direction);
		u8 x = abs(d) < 1.0f - NIGHT_EPSILON_MEDIUM;

		if (dcp.on_draw_triangle != nullptr)
		{
			real radius = is_cone ? 0 : dcp.radius;
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

				vec3 ca1 = p1 * dcp.radius + origin - direction * dcp.height;
				vec3 cb1 = p1 * radius + origin + direction * dcp.height;
				vec3 ca2 = p2 * dcp.radius + origin - direction * dcp.height;
				vec3 cb2 = p2 * radius + origin + direction * dcp.height;

				vec3 n = math::normalize(-math::cross(ca2 - ca1, cb2 - ca2));
				if (!camera.should_cull_plane(ca1, n))
				{
					Triangle tri1;
					tri1.vertices[0].point = vec4(ca1, 1);
					tri1.vertices[1].point = vec4(cb2, 1);
					tri1.vertices[2].point = vec4(cb1, 1);
					tri1.vertices[0].color = dcp.color;
					tri1.vertices[1].color = dcp.color;
					tri1.vertices[2].color = dcp.color;

					Triangle tri2;
					tri2.vertices[0].point = vec4(cb2, 1);
					tri2.vertices[1].point = vec4(ca1, 1);
					tri2.vertices[2].point = vec4(ca2, 1);
					tri2.vertices[0].color = dcp.color;
					tri2.vertices[1].color = dcp.color;
					tri2.vertices[2].color = dcp.color;

					dcp.on_draw_triangle({ .triangle = tri1, .normal = n });
					dcp.on_draw_triangle({ .triangle = tri2, .normal = n });
				}

				if (!cull_cap_a)
				{
					Triangle cap_a_tri;
					vec3 ca0 = origin - direction * dcp.height;
					cap_a_tri.vertices[0].point = vec4(ca0, 1);
					cap_a_tri.vertices[1].point = vec4(ca2, 1);
					cap_a_tri.vertices[2].point = vec4(ca1, 1);
					cap_a_tri.vertices[0].color = dcp.color;
					cap_a_tri.vertices[1].color = dcp.color;
					cap_a_tri.vertices[2].color = dcp.color;

					dcp.on_draw_triangle({ .triangle = cap_a_tri, .normal = -direction });
				}

				if (!cull_cap_b)
				{
					Triangle cap_b_tri;
					vec3 cb0 = origin + direction * dcp.height;
					cap_b_tri.vertices[0].point = vec4(cb0, 1);
					cap_b_tri.vertices[1].point = vec4(cb1, 1);
					cap_b_tri.vertices[2].point = vec4(cb2, 1);
					cap_b_tri.vertices[0].color = dcp.color;
					cap_b_tri.vertices[1].color = dcp.color;
					cap_b_tri.vertices[2].color = dcp.color;

					dcp.on_draw_triangle({ .triangle = cap_b_tri, .normal = direction });
				}

			}
		}

		if (dcp.on_draw_line == nullptr)
		{
			return;
		}

		// draw outer edges:
		if (x && backface_plane.normal != ORIGIN && dcp.on_draw_line != nullptr)
		{
			{
				DrawLineParams dlp;
				dlp.p1 = backface_plane.edge_1_a;
				dlp.p2 = backface_plane.edge_1_b;
				dlp.color = dcp.color;
				dlp.width = dcp.width;

				dcp.on_draw_line({ .params = dlp });
			}

			{
				DrawLineParams dlp;
				dlp.p1 = backface_plane.edge_2_a;
				dlp.p2 = backface_plane.edge_2_b;
				dlp.color = dcp.color;
				dlp.width = dcp.width;

				dcp.on_draw_line({ .params = dlp });
			}
		}

		vec3 up_on_cap_plane = ftd * vec4(UP, 1);
		mat4 inv_ftd = math::inverse(ftd);
		vec3 inv_pn = inv_ftd * vec4(backface_plane.normal, 1);

		// when the base encompasses the outer edges:
		auto draw_cap_ff = [&](vec3 const& cap_origin, real const& cap_radius, vec3 const& cap_normal)
			{
				for (s32 i = 0; i < segments; i++)
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
					dlp.color = dcp.color;
					dlp.width = dcp.width;

					dcp.on_draw_line({ .params = dlp });
				}
			};

		// when the base is not encompassing the outer edges
		auto draw_cap_bf = [&](vec3 const& cap_origin, real const& cap_radius, vec3 const& cap_normal, u8 culled)
			{
				if (dcp.far_corner_opacity != 0)
				{
					// calculate range of theta we will be drawing the cap in:
					auto ip = math::line_of_intersection_between_to_planes(cap_origin, -cap_normal, backface_plane.edge_1_a, backface_plane.normal);
					auto sc = Raycast3D<>::sphere(ip.origin, ip.direction, cap_origin, cap_radius);

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

						if (math::orientation(c1, c2, inv_pn) == EOrientation::CounterClockwise)
						{
							SWAP(c1, c2);
						}

						bmin_theta = math::angle_clockwise(c1, vec2(UP));
						bmax_theta = bmin_theta + math::angle_clockwise(c2, c1);

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
						// back side:
						{
							real t1 = (real)i / (real)(segments);
							t1 = math::lerp(bmin_theta, bmax_theta, t1);

							real t2 = (real)(i + 1) / (real)(segments);
							t2 = math::lerp(bmin_theta, bmax_theta, t2);

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

							if (culled)
							{
								dlp.color = dcp.color.opaqued(dcp.far_corner_opacity);
							}
							else
							{
								dlp.color = dcp.color;
							}

							dlp.width = dcp.width;

							dcp.on_draw_line({ .params = dlp });
						}

						// front side:
						{
							real t1 = (real)i / (real)(segments);
							t1 = math::lerp(fmin_theta, fmax_theta, t1);

							real t2 = (real)(i + 1) / (real)(segments);
							t2 = math::lerp(fmin_theta, fmax_theta, t2);

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

							if (culled)
							{
								dlp.color = dcp.color;
							}
							else
							{
								dlp.color = dcp.color.opaqued(dcp.near_corner_opacity);
							}

							dlp.width = dcp.width;

							dcp.on_draw_line({ .params = dlp });
						}
					}
				}
			};

			if (backface_plane.normal == ORIGIN)
			{
				draw_cap_ff(cap_a, dcp.radius, direction);
				if (!is_cone)
				{
					draw_cap_ff(cap_b, dcp.radius, direction); // TODO: may not want to do this.
				}
			}
			else
			{
				if (cull_cap_a)
				{
					draw_cap_bf(cap_a, dcp.radius, direction, true);
				}
				else
				{
					draw_cap_ff(cap_a, dcp.radius, direction);
				}

				if (!is_cone)
				{
					if (cull_cap_b)
					{
						draw_cap_bf(cap_b, dcp.radius, direction, true);
					}
					else
					{
						draw_cap_ff(cap_b, dcp.radius, direction);
					}
				}
			}

		//if (backface_plane.normal == ORIGIN)
		//{
		//	draw_base_encompassing_edges(base, dcp.radius, direction);
		//}
		//else
		//{
		//	if (cull_base)
		//	{
		//		draw_cap(base, dcp.radius, direction, true);
		//	}
		//	else
		//	{
		//		draw_cap(base, dcp.radius, direction, false);
		//	}
		//}
	}

	CylinderBackfacePlane ShapeRenderer3D::cylinder_backface_plane(vec3 const& origin, vec3 const& direction, real const& radius, real const& height, Camera const& camera)
	{
		CylinderBackfacePlane result;
		vec3 cap_a_origin = origin + direction * height;
		vec3 cap_b_origin = origin - direction * height;
		vec3 cap_a_normal = direction;
		vec3 cap_b_normal = -direction;

		if (camera.type == ECameraType::Orthographic)
		{
			vec3 undesired_angle = direction * math::dot(-camera.direction(), direction);
			vec3 desired_angle = math::normalize(-camera.direction() - undesired_angle);
			vec3 normal_perp = math::normalize(math::cross(desired_angle, direction));

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
			result.normal = -math::cross(result.edge_1_b - result.edge_1_a, result.edge_2_a - result.edge_1_a); // TODO: make sure this faces the camera

			return result;
#endif
		}

		return {};
	}

	void ShapeRenderer3D::draw_cylinder2(RenderTarget render_target, DrawCylinderParams2 const& dcp)
	{
		ASSERT(render_target != nullptr);
		Camera const& camera = render_target->camera();
		vec3 camera_direction = camera.direction();
		DecomposedTransform<> decomp = math::decompose(dcp.transform);
		vec3 const& origin = decomp.translation;
		vec3 direction = math::normalize(decomp.rotation * (FORWARD * decomp.scale));
		CylinderBackfacePlane backface_plane = cylinder_backface_plane(decomp.translation, direction, dcp.radius, dcp.height, camera);
		_draw_tube2(render_target, origin, direction, dcp, backface_plane, false);
	}

	void ShapeRenderer3D::draw_cone2(RenderTarget render_target, DrawConeParams2 const& dcp)
	{
		ASSERT(render_target != nullptr);
		Camera const& camera = render_target->camera();
		vec3 camera_direction = camera.direction();
		DecomposedTransform<> decomp = math::decompose(dcp.transform);
		vec3 const& origin = decomp.translation;
		vec3 direction = math::normalize(decomp.rotation * (FORWARD * decomp.scale));
		ConeBackfacePlane backface_plane = _cone_backface_plane2(decomp.translation, direction, dcp.radius, dcp.height, camera);
		_draw_tube2(render_target, origin, direction, dcp, backface_plane, true);
	}

	void ShapeRenderer3D::draw_sphere2(RenderTarget render_target, DrawSphereParams2 const& params)
	{
		ASSERT(render_target != nullptr);
		Camera const& camera = render_target->camera();
		//vec3 camera_direction = camera.direction();
		DecomposedTransform<> decomp = math::decompose(params.transform);
		vec3 const& origin = decomp.translation;
		vec3 direction = math::normalize(decomp.rotation * (FORWARD * decomp.scale));
		auto backface_plane = sphere_backface_plane(decomp.translation, params.radius, camera);
		s32 segments = (s32)(24.0f * params.resolution);

		auto fn = [&](real u, real v) -> Vertex<>
			{
				Vertex<> result;
				real r = sin(R_PI * v);
				result.point.x = (r * cos(2.0f * R_PI * u));
				result.point.y = (r * sin(2.0f * R_PI * u));
				result.point.z = (cos(R_PI * v));
				result.point.w = 1.0f;

				result.color = params.color;

				result.texture_coord.x = u;
				result.texture_coord.y = v;
				return result;
			};

		// TODO: add sphere rendering capabilities to renderer, a quad that uses sqrt in fragment shader to calc depth.
		
		if (params.on_draw_triangle != nullptr)
		{
			for (s32 i = 0; i < segments; i++)
			{
				for (s32 j = 0; j < segments; j++)
				{
					real u0 = (real)i / (real)segments;
					real u1 = (real)(i + 1) / (real)segments;
					real v0 = (real)j / (real)segments;
					real v1 = (real)(j + 1) / real(segments);

					Triangle<> tri_1;
					Triangle<> tri_2;

					tri_1.vertices[0] = fn(u0, v0);
					tri_1.vertices[1] = fn(u1, v0);

					Vertex<> v = fn(u0, v1);
					tri_1.vertices[2] = v;

					//vec3 normal_1 = math::normalize((vec3)tri_1.vertices[0].point + (vec3)tri_1.vertices[1].point + (vec3)tri_1.vertices[2].point);
					//vec3 normal_1 = normalize(-math::cross(vec3(tri_1.vertices[1].point) - vec3(tri_1.vertices[0].point), vec3(tri_1.vertices[2].point) - vec3(tri_1.vertices[1].point)));
					
					vec3 normal_1 = -math::cross(vec3(tri_1.vertices[1].point) - vec3(tri_1.vertices[0].point), vec3(tri_1.vertices[2].point) - vec3(tri_1.vertices[1].point));
					real len_1 = math::length(normal_1);
					normal_1 /= len_1;

					tri_1.vertices[0].point.x = tri_1.vertices[0].point.x * params.radius + origin.x;
					tri_1.vertices[0].point.y = tri_1.vertices[0].point.y * params.radius + origin.y;
					tri_1.vertices[0].point.z = tri_1.vertices[0].point.z * params.radius + origin.z;
					tri_1.vertices[1].point.x = tri_1.vertices[1].point.x * params.radius + origin.x;
					tri_1.vertices[1].point.y = tri_1.vertices[1].point.y * params.radius + origin.y;
					tri_1.vertices[1].point.z = tri_1.vertices[1].point.z * params.radius + origin.z;
					tri_1.vertices[2].point.x = tri_1.vertices[2].point.x * params.radius + origin.x;
					tri_1.vertices[2].point.y = tri_1.vertices[2].point.y * params.radius + origin.y;
					tri_1.vertices[2].point.z = tri_1.vertices[2].point.z * params.radius + origin.z;

					if (len_1 > NIGHT_EPSILON_MEDIUM && !camera.should_cull_plane(vec3(tri_1.vertices[0].point), normal_1))
					{
						params.on_draw_triangle({ .triangle = tri_1, .normal = normal_1 });
					}

					tri_2.vertices[0] = v;
					tri_2.vertices[1] = fn(u1, v0);
					tri_2.vertices[2] = fn(u1, v1);

					//vec3 normal_2 = math::normalize((vec3)tri_2.vertices[0].point + (vec3)tri_2.vertices[1].point + (vec3)tri_2.vertices[2].point);
					// vec3 normal_2 = math::normalize(-math::cross(vec3(tri_2.vertices[1].point) - vec3(tri_2.vertices[0].point), vec3(tri_2.vertices[2].point) - vec3(tri_2.vertices[1].point)));

					vec3 normal_2 = -math::cross(vec3(tri_2.vertices[1].point) - vec3(tri_2.vertices[0].point), vec3(tri_2.vertices[2].point) - vec3(tri_2.vertices[1].point));
					real len_2 = math::length(normal_2);
					normal_2 /= len_2;
					
					tri_2.vertices[0].point.x = tri_2.vertices[0].point.x * params.radius + origin.x;
					tri_2.vertices[0].point.y = tri_2.vertices[0].point.y * params.radius + origin.y;
					tri_2.vertices[0].point.z = tri_2.vertices[0].point.z * params.radius + origin.z;
					tri_2.vertices[1].point.x = tri_2.vertices[1].point.x * params.radius + origin.x;
					tri_2.vertices[1].point.y = tri_2.vertices[1].point.y * params.radius + origin.y;
					tri_2.vertices[1].point.z = tri_2.vertices[1].point.z * params.radius + origin.z;
					tri_2.vertices[2].point.x = tri_2.vertices[2].point.x * params.radius + origin.x;
					tri_2.vertices[2].point.y = tri_2.vertices[2].point.y * params.radius + origin.y;
					tri_2.vertices[2].point.z = tri_2.vertices[2].point.z * params.radius + origin.z;

					if (len_2 > NIGHT_EPSILON_MEDIUM && !camera.should_cull_plane(vec3(tri_2.vertices[0].point), normal_2))
					{
						params.on_draw_triangle({ .triangle = tri_2, .normal = normal_2 });
					}

					//vec3 n = -math::cross(vec3(tri_1.vertices[1].point) - vec3(tri_1.vertices[0].point), vec3(tri_1.vertices[2].point) - vec3(tri_1.vertices[1].point));
					//real len = math::length(n);
					//if (len < NIGHT_MATH_EPSILON)
					//{
					//	n = FORWARD;
					//}
					//else
					//{
					//	n /= len;
					//}
					
					//if (camera.should_cull_plane(vec3(tri_1.vertices[0].point), n))
					//{
					//	continue;
					//}
					//
					//tri_2.vertices[1] = fn(u1, v0);
					//tri_2.vertices[2] = fn(u1, v1);
					//
					//params.on_draw_triangle({ .triangle = tri_1, .normal = n });
					//params.on_draw_triangle({ .triangle = tri_2, .normal = n });

					//if (!params.wireframe)
					//{
					//	out_graph.draw_triangle(tri_1);
					//	out_graph.draw_triangle(tri_2);
					//}
					//else if (!params.outline_only)
					//{
					//	out_graph.draw_line(tri_1.vertices[0].point, tri_1.vertices[1].point, params.color, params.width);
					//	out_graph.draw_line(tri_1.vertices[1].point, tri_1.vertices[2].point, params.color, params.width);
					//	out_graph.draw_line(tri_1.vertices[2].point, tri_1.vertices[0].point, params.color, params.width);
					//}
				}
			}
		}

		if (params.on_draw_line != nullptr)
		{
			vec3 const& eye_location = camera.translation;

			auto [ellipse_origin, ellipse_normal, ellipse_radius] = ShapeRenderer3D::sphere_backface_plane(origin, params.radius, camera);

			mat4 forward_to_ellipse_normal = math::rotate_about_vector(FORWARD, ellipse_normal);

			DrawLineParams dlp;
			dlp.color = params.color;
			dlp.width = params.width;

			s32 line_segments = segments * 2;

			for (s32 i = 0; i < line_segments; i++)
			{
				real t1 = (real)i / (real)(line_segments - 1) * R_PI * 2;
				real t2 = (real)(i + 1) / (real)(line_segments - 1) * R_PI * 2;

				dlp.p1.x = cos(t1);
				dlp.p1.y = sin(t1);
				dlp.p1.z = 0;

				dlp.p2.x = cos(t2);
				dlp.p2.y = sin(t2);
				dlp.p2.z = 0;

				dlp.p1 *= ellipse_radius;
				dlp.p2 *= ellipse_radius;
				dlp.p1 = forward_to_ellipse_normal * vec4(dlp.p1, 1);
				dlp.p2 = forward_to_ellipse_normal * vec4(dlp.p2, 1);
				dlp.p1 += ellipse_origin;
				dlp.p2 += ellipse_origin;

				params.on_draw_line({ .params = dlp });
			}
		}
	}

#if 0
	void ShapeRenderer3D::draw_convex(DrawConvexParams const& params)
	{
		NIGHT_PROFILER_SCOPED("ShapeRenderer3D::draw_convex");
		ASSERT(params.planes != nullptr);
		vector<Plane<>> planes = *params.planes;

		ASSERT(params.out_graph != nullptr); // TODO: draw to renderer if nulltr
		RenderGraph& out_graph = *params.out_graph;
		auto crt = out_graph.current_render_target();
		ASSERT(crt != nullptr);
		vec3 camera_direction = crt->camera().direction();

		auto decomp = math::decompose(params.transform);

		for (s32 i = 0; i < planes.size(); i++)
		{
			Plane<>& plane = planes[i];

			plane.normal = Camera::triangle_normal(plane.vertices[0], plane.vertices[1], plane.vertices[2]);
			plane.normal *= decomp.scale;
			plane.normal = decomp.rotation * plane.normal;
			plane.normal = math::normalize(plane.normal);

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
#endif

}