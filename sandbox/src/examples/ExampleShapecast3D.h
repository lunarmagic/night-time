#pragma once

#include "ExampleBase.h"
#include "shape_renderer/ShapeRenderer.h"
#include "camera/Camera.h"

namespace night
{

	struct ExampleShapecast3D : public ExampleBase
	{
		// TODO: add capsule
		enum struct EShape : s32
		{
			Point = 0,
			Triangle,
			Box,
			Sphere,
			Pyramid,
			Cylinder,
			Cone,
			Max
		};

		struct Shape
		{
			EShape type = EShape::Box;
			vec3 position = ORIGIN;
			vec3 rotation = ORIGIN;
			mat4 transform = mat4(1);
			real radius = 0.75f;
			real height = 1.333f;
			vec3 rotation_origin = ORIGIN;
		};

		ExampleShapecast3D()
			: ExampleBase(ECameraType::Perspective)
		{
			reset_shapes();

			_triangleVertices[0] = vec3{
				cos(math::lerp(0.0f, R_PI * 2, 1.0f / 3)),
				sin(math::lerp(0.0f, R_PI * 2, 1.0f / 3)),
				0.0f
			};
			_triangleVertices[1] = vec3{
				cos(math::lerp(0.0f, R_PI * 2, (1.0f / 3) * 2.0f)),
				sin(math::lerp(0.0f, R_PI * 2, (1.0f / 3) * 2.0f)),
				0.0f
			};
			_triangleVertices[2] = vec3{
				cos(R_PI * 2),
				sin(R_PI * 2),
				0.0f
			};

			_boxVertices[0] = vec3{ -1.0f, 1.0f, 1.0f };
			_boxVertices[1] = vec3{ 1.0f, 1.0f, 1.0f };
			_boxVertices[2] = vec3{ 1.0f, -1.0f, 1.0f };
			_boxVertices[3] = vec3{ -1.0f, -1.0f, 1.0f };

			_boxVertices[4] = vec3{ -1.0f, 1.0f, -1.0f };
			_boxVertices[5] = vec3{ 1.0f, 1.0f, -1.0f };
			_boxVertices[6] = vec3{ 1.0f, -1.0f, -1.0f };
			_boxVertices[7] = vec3{ -1.0f, -1.0f, -1.0f };

			_pyramidVertices[0] = vec3{ -1.0f, 1.0f, -1.0f };
			_pyramidVertices[1] = vec3{ 1.0f, 1.0f, -1.0f };
			_pyramidVertices[2] = vec3{ 1.0f, -1.0f, -1.0f };
			_pyramidVertices[3] = vec3{ -1.0f, -1.0f, -1.0f };

			_pyramidVertices[4] = vec3{ 0.0f, 0.0f, 1.0f };


			_triangleVertices2D[0] = vec2{
				cos(math::lerp(0.0f, R_PI * 2, 1.0f / 3)),
				sin(math::lerp(0.0f, R_PI * 2, 1.0f / 3))
			};
			_triangleVertices2D[1] = vec2{
				cos(math::lerp(0.0f, R_PI * 2, (1.0f / 3) * 2.0f)),
				sin(math::lerp(0.0f, R_PI * 2, (1.0f / 3) * 2.0f))
			};
			_triangleVertices2D[2] = vec2{
				cos(R_PI * 2),
				sin(R_PI * 2)
			};

			_boxVertices2D[0] = vec2{ -1.0f, 1.0f };
			_boxVertices2D[1] = vec2{ 1.0f, 1.0f };
			_boxVertices2D[2] = vec2{ 1.0f, -1.0f };
			_boxVertices2D[3] = vec2{ -1.0f, -1.0f };
		}

		template<EShape A, vec3 AD, EShape B, vec3 BD>
		void edge_case()
		{
			reset_shapes();
			_shape1.type = A;
			_shape1.rotation_origin = AD;

			_shape2.type = B;
			_shape2.rotation_origin = BD;
		}

		void edge_case_triangle_epsilon()
		{
			reset_shapes();
			_shape1.type = EShape::Point;

			_shape2.type = EShape::Triangle;
			_shape2.rotation.x = -1.059f;
			_shape2.rotation.y = -0.002f;
			_shape2.rotation.z = 0.000f;
		}

		virtual void update_gui(handle<NodeGui> gui) override
		{
			ASSERT(gui != nullptr);

			if (gui->button(_2DMode ? "3D Mode" : "2D Mode"))
			{
				_2DMode = !_2DMode;
				reset_shapes();

				if (_2DMode)
				{
					auto p = find_parent<ExampleSelect>();
					ASSERT(p != nullptr);
					p->perspective_mode(ECameraType::Orthographic);
				}
				else
				{
					auto p = find_parent<ExampleSelect>();
					ASSERT(p != nullptr);
					p->perspective_mode(ECameraType::Perspective);
				}
			}

			gui->seperator();

			gui->text("shape 1:");
			gui->text("type:");

			gui->same_line();

			if (gui->button("<"))
			{
				if (!_2DMode)
				{
					_shape1.type = EShape((s32(_shape1.type) + (s32(EShape::Max) - 1)) % (s32)EShape::Max);
				}
				else
				{
					_shape1.type = EShape((s32(_shape1.type) + (s32(EShape::Pyramid) - 1)) % (s32)EShape::Pyramid);
				}
			}
			
			gui->same_line();

			if (gui->button(">"))
			{
				if (!_2DMode)
				{
					_shape1.type = EShape((s32(_shape1.type) + 1) % (s32)EShape::Max);
				}
				else
				{
					_shape1.type = EShape((s32(_shape1.type) + 1) % (s32)EShape::Pyramid);
				}
			}

			gui->drag_vec3("position", &_shape1.position, 0.01f);
			gui->drag_vec3("rotation", &_shape1.rotation, 0.01f);

			gui->seperator();
			gui->text("shape 2:");
			gui->drag_vec3("position##xx", &_shape2.position, 0.01f);
			gui->drag_vec3("rotation##xx", &_shape2.rotation, 0.01f);

			gui->text("type:");

			gui->same_line();

			if (gui->button("<##xx"))
			{
				if (!_2DMode)
				{
					_shape2.type = EShape((s32(_shape2.type) + (s32(EShape::Max) - 1)) % (s32)EShape::Max);
				}
				else
				{
					_shape2.type = EShape((s32(_shape2.type) + (s32(EShape::Pyramid) - 1)) % (s32)EShape::Pyramid);
				}
			}

			gui->same_line();

			if (gui->button(">##xx"))
			{
				if (!_2DMode)
				{
					_shape2.type = EShape((s32(_shape2.type) + 1) % (s32)EShape::Max);
				}
				else
				{
					_shape2.type = EShape((s32(_shape2.type) + 1) % (s32)EShape::Pyramid);
				}
			}

			gui->seperator();

			gui->text("edge cases:");
			if (gui->button("reset"))
			{
				reset_shapes();
			}

			// readability:
			using es = EShape;
			using es3d = ExampleShapecast3D;
			typedef void (ExampleShapecast3D:: * FN)();

			if (!_2DMode)
			{
				FN fns[] = {
					& es3d::edge_case_triangle_epsilon,
					& es3d::edge_case<es::Cylinder, RIGHT, es::Cylinder, RIGHT>,
					& es3d::edge_case<es::Cylinder, UP, es::Cylinder, UP>,
					& es3d::edge_case<es::Cylinder, UP, es::Cylinder, FORWARD>,
					& es3d::edge_case<es::Cylinder, RIGHT, es::Cylinder, FORWARD>,
					& es3d::edge_case<es::Cylinder, RIGHT, es::Cone, RIGHT>,
					& es3d::edge_case<es::Cylinder, UP, es::Cone, UP>,
					& es3d::edge_case<es::Cylinder, UP, es::Cone, FORWARD>,
					& es3d::edge_case<es::Cylinder, FORWARD, es::Cone, RIGHT>,
					& es3d::edge_case<es::Cone, UP, es::Cone, UP>,
					& es3d::edge_case<es::Cone, LEFT, es::Cone, RIGHT>,
					& es3d::edge_case<es::Cone, LEFT, es::Cone, LEFT>,
					& es3d::edge_case<es::Cone, UP, es::Cone, LEFT>,
					& es3d::edge_case<es::Cone, UP, es::Cone, DOWN>,
					& es3d::edge_case<es::Cone, FORWARD, es::Cone, RIGHT>,
					& es3d::edge_case<es::Cylinder, UP, es::Sphere, ORIGIN>,
					& es3d::edge_case<es::Cylinder, RIGHT, es::Sphere, ORIGIN>,
					& es3d::edge_case<es::Cone, UP, es::Sphere, ORIGIN>,
					& es3d::edge_case<es::Cone, RIGHT, es::Sphere, ORIGIN>,
					& es3d::edge_case<es::Cone, DOWN, es::Sphere, ORIGIN>,
					& es3d::edge_case<es::Cylinder, UP, es::Triangle, FORWARD>,
					& es3d::edge_case<es::Cylinder, RIGHT, es::Triangle, FORWARD>,
					& es3d::edge_case<es::Cone, UP, es::Triangle, FORWARD>,
					& es3d::edge_case<es::Cone, RIGHT, es::Triangle, FORWARD>,
					& es3d::edge_case<es::Cone, FORWARD, es::Triangle, FORWARD>,
					& es3d::edge_case<es::Cylinder, UP, es::Point, FORWARD>,
					& es3d::edge_case<es::Cylinder, RIGHT, es::Point, FORWARD>,
					& es3d::edge_case<es::Cone, UP, es::Point, FORWARD>,
					& es3d::edge_case<es::Cone, RIGHT, es::Point, FORWARD>,
					& es3d::edge_case<es::Cone, FORWARD, es::Point, FORWARD>,
					& es3d::edge_case<es::Triangle, FORWARD, es::Triangle, RIGHT>,
					& es3d::edge_case<es::Triangle, FORWARD, es::Triangle, UP>,
					& es3d::edge_case<es::Triangle, UP, es::Triangle, UP>,
					& es3d::edge_case<es::Triangle, FORWARD, es::Triangle, FORWARD>, // should fail
					& es3d::edge_case<es::Triangle, RIGHT, es::Triangle, RIGHT>, // should fail
				};

				s32 prev = edge_case_index;
				gui->drag_s32("edge case", &edge_case_index, 0.1f, -1, sizeof(fns) / sizeof(*fns) - 1);

				if (edge_case_index > -1)
				{
					(this->*fns[edge_case_index])();
				}
				else if (prev != -1)
				{
					reset_shapes();
				}
			}
			else
			{
				// TODO: support 2D edge cases
				
				//FN fns[] = {
				//	&es3d::edge_case<es::Triangle, FORWARD, es::Triangle, FORWARD>,
				//};
				//
				//s32 prev = edge_case_index;
				//gui->drag_s32("edge case", &edge_case_index, 0.1f, -1, sizeof(fns) / sizeof(*fns) - 1);
				//
				//if (edge_case_index > -1)
				//{
				//	(this->*fns[edge_case_index])();
				//}
				//else if (prev != -1)
				//{
				//	reset_shapes();
				//}
			}

			real delta = utility::window().delta_time();

			if (!_2DMode)
			{
				if (utility::key_down(EKey::A))
				{
					_shape1.rotation.y += 1.0f * delta;
				}

				if (utility::key_down(EKey::D))
				{
					_shape1.rotation.y -= 1.0f * delta;
				}

				if (utility::key_down(EKey::W))
				{
					_shape1.rotation.x += 1.0f * delta;
				}

				if (utility::key_down(EKey::S))
				{
					_shape1.rotation.x -= 1.0f * delta;
				}

				if (utility::key_down(EKey::Left))
				{
					_shape2.rotation.y += 1.0f * delta;
				}

				if (utility::key_down(EKey::Right))
				{
					_shape2.rotation.y -= 1.0f * delta;
				}

				if (utility::key_down(EKey::Up))
				{
					_shape2.rotation.x += 1.0f * delta;
				}

				if (utility::key_down(EKey::Down))
				{
					_shape2.rotation.x -= 1.0f * delta;
				}
			}
			else
			{
				if (utility::key_down(EKey::A))
				{
					_shape1.rotation.z += 1.0f * delta;
				}

				if (utility::key_down(EKey::D))
				{
					_shape1.rotation.z -= 1.0f * delta;
				}

				if (utility::key_down(EKey::Left))
				{
					_shape2.rotation.z += 1.0f * delta;
				}

				if (utility::key_down(EKey::Right))
				{
					_shape2.rotation.z -= 1.0f * delta;
				}
			}

			_shape1.transform = mat4(1);
			_shape1.transform = math::quat_to_mat4(quat(_shape1.rotation)) * _shape1.transform;
			_shape1.transform = math::translate(_shape1.position) * _shape1.transform;
			
			_shape2.transform = mat4(1);
			_shape2.transform = math::quat_to_mat4(quat(_shape2.rotation)) * _shape2.transform;
			_shape2.transform = math::translate(_shape2.position) * _shape2.transform;

			auto get_support_3d = [&](Shape const& shape) -> function<vec3 (vec3 const&)>
			{
				DecomposedTransform<> dt = math::decompose(shape.transform);

				vec3 direction;

				if (shape.rotation_origin != ORIGIN)
				{
					direction = shape.rotation_origin * dt.rotation;
					dt.rotation = quat(direction * R_PI * (real)0.5);
				}
				else
				{
					direction = FORWARD * dt.rotation;
				}

				mat4 transform = math::compose(dt);

				switch (shape.type)
				{
				case EShape::Point:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return dt.translation;
						};
					break;
				}

				case EShape::Triangle:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_polygon(dir, transform, _triangleVertices.begin(), _triangleVertices.end());
						};
					break;
				}

				case EShape::Box:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_polygon(dir, shape.transform, _boxVertices.begin(), _boxVertices.end());
						};
					break;
				}
				case EShape::Pyramid:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_polygon(dir, shape.transform, _pyramidVertices.begin(), _pyramidVertices.end());
						};
					break;
				}
				case EShape::Sphere:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_sphere(dir, transform, shape.radius);
						};
					break;
				}
				case EShape::Cylinder:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_cylinder(dir, transform, shape.radius, shape.height);
						};
					break;
				}
				case EShape::Cone:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_cone(dir, transform, shape.radius, shape.height);
						};
					break;
				}
				}

				return nullptr;
			};

			auto get_support_2d = [&](Shape const& shape) -> function<vec2(vec2 const&)>
				{
					DecomposedTransform<> dt = math::decompose(shape.transform);

					vec3 direction;

					if (shape.rotation_origin != ORIGIN)
					{
						direction = shape.rotation_origin * dt.rotation;
						dt.rotation = quat(direction * R_PI * (real)0.5);
					}
					else
					{
						direction = FORWARD * dt.rotation;
					}

					mat4 transform = math::compose(dt);

					switch (shape.type)
					{
					case EShape::Point:
					{
						return [=](vec2 const& dir) -> vec2
							{
								return dt.translation;
							};
						break;
					}

					case EShape::Triangle:
					{
						return [=](vec2 const& dir) -> vec2
							{
								return GJK2D<>::support_polygon(dir, transform, _triangleVertices2D.begin(), _triangleVertices2D.end());
							};
						break;
					}

					case EShape::Box:
					{
						return [=](vec2 const& dir) -> vec2
							{
								return GJK2D<>::support_polygon(dir, shape.transform, _boxVertices2D.begin(), _boxVertices2D.end());
							};
						break;
					}

					case EShape::Sphere:
					{
						return [=](vec2 const& dir) -> vec2
							{
								return GJK2D<>::support_circle(dir, transform, shape.radius);
							};
						break;
					}
					}

					return nullptr;
				};

			if (!_2DMode)
			{
				//function<vec3(vec3 const&)> support_1 = get_support_3d(_shape1);
				//function<vec3(vec3 const&)> support_2 = get_support_3d(_shape2);

				ShapeCastParams3D<> params;
				params.support_casted = get_support_3d(_shape1);
				params.support_against = get_support_3d(_shape2);
				params.motion = _motion;
#ifdef NIGHT_USE_DOUBLE_PRECISION
				params.epsilon = NIGHT_EPSILON_SMALL_DOUBLE;
#endif

				{
					DB_ALGO_INVOLVE_NODES_SCOPED(handle_from_this());
					DB_ALGO_SCOPED("GJK Shapecast Example");
					NIGHT_PROFILER_SCOPED("GJK Shapecast Example");
					_shapeCastResult = GJK3D<>::shape_cast(params);
				}
			}
			else
			{
				ShapeCastParams2D<> params;
				params.support_casted = get_support_2d(_shape1);
				params.support_against = get_support_2d(_shape2);
				params.motion = _motion;
#ifdef NIGHT_USE_DOUBLE_PRECISION
				params.epsilon = NIGHT_EPSILON_SMALL_DOUBLE;
#endif

				{
					DB_ALGO_INVOLVE_NODES_SCOPED(handle_from_this());
					DB_ALGO_SCOPED("GJK Shapecast Example");
					NIGHT_PROFILER_SCOPED("GJK Shapecast Example");
					auto result = GJK2D<>::shape_cast(params);
					_shapeCastResult.result = result.result;
					_shapeCastResult.t0 = result.t0;
					_shapeCastResult.t1 = result.t1;
					_shapeCastResult.n0 = vec3(result.n0, 0.0f);
					_shapeCastResult.n1 = vec3(result.n1, 0.0f);
					_shapeCastResult.c0 = vec3(result.c0, 50.0f);
					_shapeCastResult.c1 = vec3(result.c1, 50.0f);
				}
			}
		}

	protected:

		virtual void on_render(RenderGraph& out_graph) const override
		{
			function<void(DrawLineCallbackParams const&)> on_draw_line;
			function<void(DrawTriangleCallbackParams const&)> on_draw_triangle;
			
			if (!_showOutline)
			{
				on_draw_line = nullptr;
			}
			else
			{
				on_draw_line = [&](DrawLineCallbackParams const& params)
					{
						DrawLineParams dlp = params.params;
						dlp.color.r = 0;
						dlp.color.g = 0;
						dlp.color.b = 0;
						out_graph.draw_line(dlp);
					};
			}

			if (!_showTriangleFill || _2DMode)
			{
				on_draw_triangle = nullptr;
			}
			else
			{
				on_draw_triangle = [&](DrawTriangleCallbackParams const& params)
					{
						Triangle triangle = params.triangle;

						real d = math::dot(params.normal, RIGHT);

						d += 1;
						d /= 2;

						triangle.vertices[0].color = triangle.vertices[0].color.darken(math::lerp(0.2f, 1.0f, d));
						triangle.vertices[1].color = triangle.vertices[1].color.darken(math::lerp(0.2f, 1.0f, d));
						triangle.vertices[2].color = triangle.vertices[2].color.darken(math::lerp(0.2f, 1.0f, d));

						out_graph.draw_triangle(triangle);
					};
			}


			auto draw_shape = [&](Shape const& shape, vec3 const& offset, Color const& color)
				{
					DecomposedTransform<> dt = math::decompose(shape.transform);
					dt.translation += offset;

					vec3 direction;

					if (shape.rotation_origin != ORIGIN)
					{
						direction = shape.rotation_origin * dt.rotation;
						dt.rotation = quat(direction * R_PI * (real)0.5);
					}
					else
					{
						direction = FORWARD * dt.rotation;
					}
					
					mat4 transform = math::compose(dt);
					
					switch (shape.type)
					{
					case EShape::Point:
					{
						// TODO: contact points
						out_graph.draw_point(dt.translation, BLACK.opaqued(0.75f));
						break;
					}

					case EShape::Triangle:
					{
						out_graph.draw_line(transform * vec4(_triangleVertices[0], 1), transform * vec4(_triangleVertices[1], 1), BLACK.opaqued(0.75f));
						out_graph.draw_line(transform * vec4(_triangleVertices[1], 1), transform * vec4(_triangleVertices[2], 1), BLACK.opaqued(0.75f));
						out_graph.draw_line(transform * vec4(_triangleVertices[2], 1), transform * vec4(_triangleVertices[0], 1), BLACK.opaqued(0.75f));

						break;
					}

					case EShape::Box:
					{
						DrawBoxParams dbp;
						dbp.transform = transform;
						dbp.color = color;
						dbp.on_draw_line = on_draw_line;
						dbp.on_draw_triangle = on_draw_triangle;
						ShapeRenderer3D::draw_box(out_graph.current_render_target(), dbp);

						break;
					}

					case EShape::Pyramid:
					{
						DrawPyramidParams dpp;
						dpp.transform = transform;
						dpp.color = color;
						dpp.on_draw_line = on_draw_line;
						dpp.on_draw_triangle = on_draw_triangle;
						ShapeRenderer3D::draw_pyramid(out_graph.current_render_target(), dpp);

						break;
					}

					case EShape::Sphere:
					{
						DrawSphereParams2 dsp;
						dsp.transform = transform;
						dsp.radius = shape.radius;
						dsp.color = color;
						dsp.on_draw_line = on_draw_line;
						dsp.on_draw_triangle = on_draw_triangle;
						ShapeRenderer3D::draw_sphere2(out_graph.current_render_target(), dsp);
						break;
					}

					case EShape::Cylinder :
					{
						DrawCylinderParams2 dcp;
						dcp.transform = transform;
						dcp.radius = shape.radius;
						dcp.height = shape.height;
						dcp.color = color;
						dcp.on_draw_line = on_draw_line;
						dcp.on_draw_triangle = on_draw_triangle;
						ShapeRenderer3D::draw_cylinder2(out_graph.current_render_target(), dcp);
						break;
					}

					case EShape::Cone:
					{
						DrawConeParams2 dcp;
						dcp.transform = transform;
						dcp.radius = shape.radius;
						dcp.height = shape.height;
						dcp.color = color;
						dcp.on_draw_line = on_draw_line;
						dcp.on_draw_triangle = on_draw_triangle;
						ShapeRenderer3D::draw_cone2(out_graph.current_render_target(), dcp);
						break;
					}
					}
				};

			draw_shape(_shape1, ORIGIN, GREY.opaqued(0.75f));

			if (_shapeCastResult.result)
			{
				draw_shape(_shape1, _motion * _shapeCastResult.t0, RED.opaqued(0.75f));
				draw_shape(_shape1, _motion * _shapeCastResult.t1, BLUE.opaqued(0.5f));

				if (_2DMode) // TODO: support contacts for 3D
				{
					out_graph.draw_line(_shapeCastResult.c0, _shapeCastResult.c0 + _shapeCastResult.n0, RED);
					out_graph.draw_line(_shapeCastResult.c1, _shapeCastResult.c1 + _shapeCastResult.n1, BLUE);
				}
			}
			
			draw_shape(_shape2, ORIGIN, GREY.opaqued(0.76f));
		}

	private:

		void reset_shapes()
		{
			_shape1 = {};
			_shape2 = {};
			_shape1.position = vec3(-2.0f, 0.0f, 0.0f);
			_shape2.position = vec3(2.0f, 0.0f, 0.0f);
			_shape1.type = EShape::Box;
			_shape2.type = EShape::Sphere;
			edge_case_index = -1;
		}

		Shape _shape1 = {};
		Shape _shape2 = {};
		vec3 _motion = RIGHT;
		ShapeCastResult3D<> _shapeCastResult = { .result = false };
		array<vec3, 3> _triangleVertices;
		array<vec3, 8> _boxVertices;
		array<vec3, 5> _pyramidVertices;

		b8 _showOutline = true;
		b8 _showTriangleFill = true;
		s32 edge_case_index = -1;

		b8 _2DMode = false;
		array<vec2, 3> _triangleVertices2D;
		array<vec2, 4> _boxVertices2D;
	};

}