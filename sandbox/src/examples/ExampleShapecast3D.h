#pragma once

#include "ExampleBase.h"
#include "shape_renderer/ShapeRenderer.h"
#include "camera/Camera.h"

namespace night
{

	struct ExampleShapecast3D : public ExampleBase
	{
		// TODO: add capsule
		enum EShape
		{
			Point = 0,
			Triangle,
			Box,
			Pyramid,
			Sphere,
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
			array<vec3, 9> points; // index 0-7 is box vertices, 4-9 is pyramid 
			real radius = 0.75f;
			real height = 1.333f;
			vec3 rotation_origin = FORWARD;
		};

		ExampleShapecast3D()
		{
			_shape1.position = vec3(-2.0f, 0.0f, 0.0f);
			//_shape1.rotation = vec3(0.46f, 1.52f, 5.6f);
			_shape1.type = EShape::Cone;

			_shape2.position = vec3(2.0f, 0.0f, 0.0f);
			//_shape2.rotation = vec3(2.46f, 4.1222f, 3.6111f);
			_shape2.type = EShape::Cone;

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
		}

	protected:

		virtual void update_gui(handle<NodeGui> gui) override
		{
			ASSERT(gui != nullptr);
			
			// TODO: make reset function.
			auto reset_shapes = [&]()
				{
					_shape1 = {};
					_shape2 = {};
					_shape1.position = vec3(-2.0f, 0.0f, 0.0f);
					_shape2.position = vec3(2.0f, 0.0f, 0.0f);
					_shape1.type = EShape::Cone;
					_shape2.type = EShape::Cone;
				};

			gui->seperator();

			gui->text("shape 1:");
			gui->text("type:");

			gui->same_line();

			if (gui->button("<"))
			{
				_shape1.type = EShape((s32(_shape1.type) + s32(EShape::Max - 1)) % (s32)EShape::Max);
			}
			
			gui->same_line();

			if (gui->button(">"))
			{
				_shape1.type = EShape((s32(_shape1.type) + 1) % (s32)EShape::Max);
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
				_shape2.type = EShape((s32(_shape2.type) + s32(EShape::Max - 1)) % (s32)EShape::Max);
			}

			gui->same_line();

			if (gui->button(">##xx"))
			{
				_shape2.type = EShape((s32(_shape2.type) + 1) % (s32)EShape::Max);
			}

			gui->seperator();

#pragma region EDGE_CASES
			gui->text("edge cases:");
			if (gui->button("reset"))
			{
				reset_shapes();
			}
			else if (gui->button("triangles orthogonal 1"))
			{
				reset_shapes();
				_shape1.type = EShape::Triangle;
				_shape1.rotation_origin = FORWARD;

				_shape2.type = EShape::Triangle;
				_shape2.rotation_origin = RIGHT;
			}
			else if (gui->button("triangles orthogonal 2"))
			{
				reset_shapes();
				_shape1.type = EShape::Triangle;
				_shape1.rotation_origin = FORWARD;

				_shape2.type = EShape::Triangle;
				_shape2.rotation_origin = UP;
			}
			else if (gui->button("point-triangle epsilon"))
			{
				reset_shapes();
				_shape1.type = EShape::Point;

				_shape2.type = EShape::Triangle;
				_shape2.rotation.x = 1.509f;
				_shape2.rotation.y = 1.285f;
				_shape2.rotation.z = 0.0f;
			}
			else if (gui->button("cylinders parallel"))
			{
				reset_shapes();
				_shape1.type = EShape::Cylinder;
				_shape1.rotation_origin = RIGHT;

				_shape2.type = EShape::Cylinder;
				_shape2.rotation_origin = RIGHT;
			}
			else if (gui->button("cylinders adjacent"))
			{
				reset_shapes();
				_shape1.type = EShape::Cylinder;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cylinder;
				_shape2.rotation_origin = UP;
			}
			else if (gui->button("cylinders orthogonal"))
			{
				reset_shapes();
				_shape1.type = EShape::Cylinder;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cylinder;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("cone-cylinder parallel"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = LEFT;

				_shape2.type = EShape::Cylinder;
				_shape2.rotation_origin = RIGHT;
			}
			else if (gui->button("cone-cylinder adjacent"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cylinder;
				_shape2.rotation_origin = UP;
			}
			else if (gui->button("cone-cylinder orthogonal"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cylinder;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("cones parallel"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = RIGHT;

				_shape2.type = EShape::Cone;
				_shape2.rotation_origin = RIGHT;
			}
			else if (gui->button("cones base-base"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = RIGHT;

				_shape2.type = EShape::Cone;
				_shape2.rotation_origin = LEFT;
			}
			else if (gui->button("cones tip-tip"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = LEFT;

				_shape2.type = EShape::Cone;
				_shape2.rotation_origin = RIGHT;
			}
			else if (gui->button("cones adjacent"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cone;
				_shape2.rotation_origin = UP;
			}
			else if (gui->button("cones opposite"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cone;
				_shape2.rotation_origin = DOWN;
			}
			else if (gui->button("cones orthogonal"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Cone;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("cylinder-sphere parallel"))
			{
				reset_shapes();
				_shape1.type = EShape::Cylinder;
				_shape1.rotation_origin = LEFT;

				_shape2.type = EShape::Sphere;
				_shape2.rotation_origin = FORWARD;
				}
			else if (gui->button("cylinder-sphere adjacent"))
			{
				reset_shapes();
				_shape1.type = EShape::Cylinder;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Sphere;
				_shape2.rotation_origin = FORWARD;
				}
			else if (gui->button("cone-sphere parallel"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = LEFT;

				_shape2.type = EShape::Sphere;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("cone-sphere adjacent"))
			{
				reset_shapes();
				_shape1.type = EShape::Cone;
				_shape1.rotation_origin = UP;

				_shape2.type = EShape::Sphere;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("sphere-sphere parallel"))
			{
				reset_shapes();
				_shape1.type = EShape::Sphere;
				_shape1.rotation_origin = FORWARD;

				_shape2.type = EShape::Sphere;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("triangles parallel (should fail)"))
			{
				reset_shapes();
				_shape1.type = EShape::Triangle;
				_shape1.rotation_origin = FORWARD;

				_shape2.type = EShape::Triangle;
				_shape2.rotation_origin = FORWARD;
			}
			else if (gui->button("point triangle parallel (should fail)"))
			{
				reset_shapes();
				_shape1.type = EShape::Point;
				_shape1.rotation_origin = FORWARD;

				_shape2.type = EShape::Triangle;
				_shape2.rotation_origin = FORWARD;
			}

#pragma endregion

			real delta = utility::window().delta_time();

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

			_shape1.transform = mat4(1);
			_shape1.transform = math::quat_to_mat4(quat(_shape1.rotation)) * _shape1.transform;
			_shape1.transform = math::translate(_shape1.position) * _shape1.transform;
			
			_shape2.transform = mat4(1);
			_shape2.transform = math::quat_to_mat4(quat(_shape2.rotation)) * _shape2.transform;
			_shape2.transform = math::translate(_shape2.position) * _shape2.transform;

			auto get_support = [&](Shape const& shape) -> function<vec3 (vec3 const&)>
			{
				DecomposedTransform<> dt = math::decompose(shape.transform);

				vec3 direction = shape.rotation_origin * dt.rotation;
				dt.rotation = quat(direction * R_PI * 0.5f);
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
							return GJK3D<>::support_polygon(dir, shape.transform, shape.points.begin(), shape.points.end());
						};
					break;
				}
				case EShape::Pyramid:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_polygon(dir, shape.transform, shape.points.begin(), shape.points.end());
						};
					break;
				}
				case EShape::Sphere:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_sphere(dir, dt.translation, shape.radius);
						};
					break;
				}
				case EShape::Cylinder:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_cylinder(dir, dt.translation, direction, shape.radius, shape.height);
						};
					break;
				}
				case EShape::Cone:
				{
					return [=](vec3 const& dir) -> vec3
						{
							return GJK3D<>::support_cone(dir, dt.translation, direction, shape.radius, shape.height);
						};
					break;
				}
				}

				return nullptr;
			};

			function<vec3 (vec3 const&)> support_1 = get_support(_shape1);
			function<vec3 (vec3 const&)> support_2 = get_support(_shape2);

			ShapeCastParams3D<> params;
			params.support_casted = get_support(_shape1);
			params.support_against = get_support(_shape2);
			params.motion = _motion;

			{
				DB_ALGO_INVOLVE_NODES_SCOPED(handle_from_this());
				DB_ALGO_SCOPED("GJK Shapecast Example");
				NIGHT_PROFILER_SCOPED("GJK Shapecast Example");
				_shapeCastResult = GJK3D<>::shape_cast(params);
			}
		}

		virtual void on_render(RenderGraph& out_graph) const override
		{
			auto draw_shape = [&](Shape const& shape, vec3 const& offset, Color const& color)
				{
					DecomposedTransform<> dt = math::decompose(shape.transform);
					dt.translation += offset;

					vec3 direction = shape.rotation_origin * dt.rotation;
					dt.rotation = quat(direction * R_PI * 0.5f);
					mat4 transform = math::compose(dt);
					
					switch (shape.type)
					{
					case EShape::Point:
					{
						// TODO: contact points
						out_graph.draw_point(dt.translation, color);

						//if (offset != ORIGIN)
						//{
						//	out_graph.draw_line(dt.translation - offset, dt.translation, color.opaqued(0.5f));
						//}

						break;
					}

					case EShape::Triangle:
					{
						out_graph.draw_line(transform * vec4(_triangleVertices[0], 1), transform * vec4(_triangleVertices[1], 1), color);
						out_graph.draw_line(transform * vec4(_triangleVertices[1], 1), transform * vec4(_triangleVertices[2], 1), color);
						out_graph.draw_line(transform * vec4(_triangleVertices[2], 1), transform * vec4(_triangleVertices[0], 1), color);
						break;
					}

					case EShape::Box:
					{
						break;
					}

					case EShape::Pyramid:
					{
						break;
					}

					case EShape::Sphere:
					{
						DrawSphereParams dsp;
						dsp.origin = dt.translation;
						dsp.radius = shape.radius;
						dsp.color = color;
						dsp.outline_only = true;
						dsp.out_graph = &out_graph;
						ShapeRenderer::draw_sphere(dsp);
						break;
					}

					case EShape::Cylinder :
					{
						DrawCylinderParams dcp;
						dcp.origin = dt.translation;
						dcp.direction = direction;
						dcp.radius = shape.radius;
						dcp.height = shape.height;
						dcp.color = color;
						dcp.out_graph = &out_graph;
						ShapeRenderer::draw_cylinder(dcp);
						break;
					}

					case EShape::Cone:
					{
						DrawConeParams dcp;
						dcp.origin = dt.translation;
						dcp.direction = direction;
						dcp.radius = shape.radius;
						dcp.height = shape.height;
						dcp.color = color;
						dcp.out_graph = &out_graph;
						ShapeRenderer::draw_cone(dcp);
						break;
					}
					}
				};

			draw_shape(_shape1, ORIGIN, BLACK.opaqued(0.75f));

			if (_shapeCastResult.result)
			{
				draw_shape(_shape1, _motion * _shapeCastResult.t0, RED.opaqued(0.75f));
				draw_shape(_shape1, _motion * _shapeCastResult.t1, BLUE.opaqued(0.5f));
			}
			
			draw_shape(_shape2, ORIGIN, BLACK.opaqued(0.75f));
		}

	private:

		Shape _shape1 = {};
		Shape _shape2 = {};
		vec3 _motion = RIGHT;
		ShapeCastResult3D<> _shapeCastResult = { .result = false };
		array<vec3, 3> _triangleVertices;
	};

}