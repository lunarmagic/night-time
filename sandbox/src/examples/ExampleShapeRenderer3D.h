#pragma once

#include "ExampleBase.h"
#include "shape_renderer/ShapeRenderer.h"
#include "camera/Camera.h"

namespace night
{

	struct ExampleShapeRenderer3D : public ExampleBase
	{
		// TODO: add capsule
		enum struct EShape : s32
		{
			//Triangle,
			Box = 0,
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
			real radius = 0.75f;
			real height = 1.333f;
		};

		ExampleShapeRenderer3D()
			: ExampleBase(ECameraType::Perspective)
		{
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
		}

	protected:

		virtual void update_gui(handle<NodeGui> gui) override
		{
			ASSERT(gui != nullptr);

			gui->checkbox("show outline", &_showOutline);
			gui->checkbox("show triangle fill", &_showTriangleFill);

			gui->seperator();

			gui->text("shape 1:");
			gui->text("type:");

			gui->same_line();

			if (gui->button("<"))
			{
				_shape.type = EShape((s32(_shape.type) + (s32(EShape::Max) - 1)) % (s32)EShape::Max);
			}

			gui->same_line();

			if (gui->button(">"))
			{
				_shape.type = EShape((s32(_shape.type) + 1) % (s32)EShape::Max);
			}

			gui->drag_vec3("position", &_shape.position, 0.01f);
			gui->drag_vec3("rotation", &_shape.rotation, 0.01f);

			gui->seperator();

#pragma region EDGE_CASES
			gui->text("edge cases:");
			if (gui->button("reset"))
			{
				_shape = {};
			}
#pragma endregion

			real delta = utility::window().delta_time();

			if (utility::key_down(EKey::D))
			{
				_shape.position.x += 1.0f * delta;
			}

			if (utility::key_down(EKey::A))
			{
				_shape.position.x -= 1.0f * delta;
			}

			if (utility::key_down(EKey::W))
			{
				_shape.position.y += 1.0f * delta;
			}

			if (utility::key_down(EKey::S))
			{
				_shape.position.y -= 1.0f * delta;
			}

			if (utility::key_down(EKey::Left))
			{
				_shape.rotation.y += 1.0f * delta;
			}

			if (utility::key_down(EKey::Right))
			{
				_shape.rotation.y -= 1.0f * delta;
			}

			if (utility::key_down(EKey::Up))
			{
				_shape.rotation.x += 1.0f * delta;
			}

			if (utility::key_down(EKey::Down))
			{
				_shape.rotation.x -= 1.0f * delta;
			}

			_shape.transform = mat4(1);
			_shape.transform = math::quat_to_mat4(quat(_shape.rotation)) * _shape.transform;
			_shape.transform = math::translate(_shape.position) * _shape.transform;
		}

		virtual void on_render(RenderGraph& out_graph) const override
		{
			function<void(DrawLineCallbackParams const&)> on_draw_line;

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
			function<void(DrawTriangleCallbackParams const&)> on_draw_triangle;
			if (!_showTriangleFill)
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

						triangle.vertices[0].color = Color::lerp(BLACK, RED.opaqued(0.75f), math::lerp(0.0f, 1.0f, d));
						triangle.vertices[1].color = Color::lerp(BLACK, BLUE.opaqued(0.75f), math::lerp(0.0f, 1.0f, d));
						triangle.vertices[2].color = Color::lerp(BLACK, GREEN.opaqued(0.75f), math::lerp(0.0f, 1.0f, d));

						out_graph.draw_triangle(triangle);
					};
			}

			Color color = BLACK.opaqued(0.75f);

			DecomposedTransform<> dt = math::decompose(_shape.transform);

			switch (_shape.type)
			{
			case EShape::Box:
			{
				DrawBoxParams dbp;
				dbp.transform = _shape.transform;
				dbp.color = color;
				dbp.on_draw_line = on_draw_line;
				dbp.on_draw_triangle = on_draw_triangle;
				ShapeRenderer::draw_box(out_graph.current_render_target(), dbp);

				break;
			}

			case EShape::Pyramid:
			{
				DrawPyramidParams dpp;
				dpp.transform = _shape.transform;
				dpp.color = color;
				dpp.on_draw_line = on_draw_line;
				dpp.on_draw_triangle = on_draw_triangle;
				ShapeRenderer::draw_pyramid(out_graph.current_render_target(), dpp);

				break;
			}

			case EShape::Sphere:
			{
				DrawSphereParams2 dsp;
				dsp.transform = _shape.transform;
				dsp.radius = _shape.radius;
				dsp.color = color;
				dsp.on_draw_line = on_draw_line;
				dsp.on_draw_triangle = on_draw_triangle;
				ShapeRenderer::draw_sphere2(out_graph.current_render_target(), dsp);
				break;
			}

			case EShape::Cylinder:
			{
				DrawCylinderParams2 dcp;
				dcp.transform = _shape.transform;
				dcp.radius = _shape.radius;
				dcp.height = _shape.height;
				dcp.color = color;
				dcp.on_draw_line = on_draw_line;
				dcp.on_draw_triangle = on_draw_triangle;
				ShapeRenderer::draw_cylinder2(out_graph.current_render_target(), dcp);
				break;
			}

			case EShape::Cone:
			{
				DrawConeParams2 dcp;
				dcp.transform = _shape.transform;
				dcp.radius = _shape.radius;
				dcp.height = _shape.height;
				dcp.color = color;
				dcp.on_draw_line = on_draw_line;
				dcp.on_draw_triangle = on_draw_triangle;
				ShapeRenderer::draw_cone2(out_graph.current_render_target(), dcp);
				break;
			}
			}
		}

	private:

		Shape _shape = {};
		array<vec3, 3> _triangleVertices;
		array<vec3, 8> _boxVertices;
		array<vec3, 5> _pyramidVertices;
		u8 _showOutline = false;
		u8 _showTriangleFill = true;
	};

}