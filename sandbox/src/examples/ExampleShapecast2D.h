#pragma once

#if 0
#include "ExampleBase.h"
#include "random/random.h"
#include "geometry/Convex.h"
#include "gjk/GJK.h"

namespace night
{
	struct ExampleShapecast2D : public ExampleBase
	{
		struct Shape
		{
			vector<vec2> points;
			vec2 position;
		};

		ExampleShapecast2D()
			: ExampleBase(ECameraType::Orthographic)
		{
			reset_shapes();
		}

		void reset_shapes()
		{
			_shape1 = {};
			fill_shape_random(_shape1);
			_shape1.points = Convex2D<>::make_convex(_shape1.points.begin(), _shape1.points.end());
			_shape1.position = vec2{ -1.5f, 0.0f };

			_shape2 = {};
			fill_shape_random(_shape2);
			_shape2.position = vec2{ 1.5f, 0.0f };
			_shape2.points = Convex2D<>::make_convex(_shape2.points.begin(), _shape2.points.end());
		}

		void fill_shape_random(Shape& out_shape)
		{
			out_shape.points.clear();
			for (s32 i = 0; i < _fillSize; i++)
			{
				real t = (real)i / (real)(_fillSize - 1);
				out_shape.points.push_back(vec2{ random(_shapeSize) - _shapeSize / 2, random(_shapeSize) - _shapeSize / 2 });
			}
		}

		virtual void update_gui(handle<NodeGui> gui) override
		{
			ASSERT(gui != nullptr);

			gui->drag_s32("Size", &_fillSize, 0.1f, 0, 64);

			gui->seperator();
			gui->text("Shape 1:");

			if (Convex2D<>::is_clockwise(_shape1.points.begin(), _shape1.points.end()))
			{
				gui->text("Winding Order: Clockwise");
			}
			else
			{
				gui->text("Winding Order: Counter Clockwise");
			}

			if (gui->button("Fill Random"))
			{
				//emit_signal("TEST SIGNAL", 5.1354f);
				fill_shape_random(_shape1);
			}

			if (gui->button("Make Convex"))
			{
				_shape1.points = Convex2D<>::make_convex(_shape1.points.begin(), _shape1.points.end());
			}

			if (gui->button("Make Clockwise"))
			{
				Convex2D<>::wind_up(_shape1.points.begin(), _shape1.points.end(), EOrientation::Clockwise);
			}

			if (gui->button("Make Counter-Clockwise"))
			{
				Convex2D<>::wind_up(_shape1.points.begin(), _shape1.points.end(), EOrientation::CounterClockwise);
			}

			gui->seperator();
			gui->text("Shape 2:");

			if (Convex2D<>::is_clockwise(_shape2.points.begin(), _shape2.points.end()))
			{
				gui->text("Winding Order: Clockwise");
			}
			else
			{
				gui->text("Winding Order: Counter Clockwise");
			}

			if (gui->button("Fill Random##xx"))
			{
				fill_shape_random(_shape2);
			}

			if (gui->button("Make Convex##xx"))
			{
				_shape2.points = Convex2D<>::make_convex(_shape2.points.begin(), _shape2.points.end());
			}

			if (gui->button("Make Clockwise##xx"))
			{
				Convex2D<>::wind_up(_shape2.points.begin(), _shape2.points.end(), EOrientation::Clockwise);
			}

			if (gui->button("Make Counter-Clockwise##xx"))
			{
				Convex2D<>::wind_up(_shape2.points.begin(), _shape2.points.end(), EOrientation::CounterClockwise);
			}

			real delta = utility::window().delta_time();

			if (utility::key_down(EKey::D))
			{
				_shape1.position.x += 1.0f * delta;
			}

			if (utility::key_down(EKey::A))
			{
				_shape1.position.x -= 1.0f * delta;
			}

			if (utility::key_down(EKey::W))
			{
				_shape1.position.y += 1.0f * delta;
			}

			if (utility::key_down(EKey::S))
			{
				_shape1.position.y -= 1.0f * delta;
			}

			if (utility::key_down(EKey::Left))
			{
				_shape2.position.x += 1.0f * delta;
			}

			if (utility::key_down(EKey::Right))
			{
				_shape2.position.x -= 1.0f * delta;
			}

			if (utility::key_down(EKey::Up))
			{
				_shape2.position.y += 1.0f * delta;
			}

			if (utility::key_down(EKey::Down))
			{
				_shape2.position.y -= 1.0f * delta;
			}
		}

	protected:

		virtual void on_render(RenderGraph& out_graph) const override
		{
			auto draw_shape = [&](vector<vec2> const& points, Color const& color)
				{
					for (s32 i = 0; i < points.size(); i++)
					{
						vec2 p1 = points[i];
						vec2 p2 = points[(i + 1) % points.size()];
						out_graph.draw_line(vec3(p1, -NIGHT_EPSILON_MEDIUM), vec3(p2, -NIGHT_EPSILON_MEDIUM), color);
					}
				};

			vector<vec2> s1x = _shape1.points;
			for (auto& i : s1x) i += _shape1.position;
			vector<vec2> s2x = _shape2.points;
			for (auto& i : s2x) i += _shape2.position;

			draw_shape(s1x, BLACK.opaqued(0.75f));
			draw_shape(s2x, BLACK.opaqued(0.75f));

			IntersectsParams2D<> ip2d;
			ip2d.support_a = [&](vec2 const& dir) -> vec2
				{
					return GJK2D<>::support_polygon(dir, mat4(1), s1x.begin(), s1x.end());
				};

			ip2d.support_b = [&](vec2 const& dir) -> vec2
				{
					return GJK2D<>::support_polygon(dir, mat4(1), s2x.begin(), s2x.end());
				};

			DB_ALGO_INVOLVE_NODES_SCOPED(handle_from_this());

			ShapeCastParams2D scp2d;
			vec2 motion = RIGHT;
			scp2d.motion = motion;

			scp2d.support_casted = [&](vec2 const& dir) -> vec2 const&
				{
					return GJK2D<>::support_polygon(dir, mat4(1), s1x.begin(), s1x.end());
				};

			scp2d.support_against = [&](vec2 const& dir) -> vec2 const&
				{
					return GJK2D<>::support_polygon(dir, mat4(1), s2x.begin(), s2x.end());
				};

			auto sc = GJK2D<>::shape_cast(scp2d, false);

			if (sc.result)
			{
				vector<vec2> s1xx = s1x;
				for (auto& i : s1xx) i += scp2d.motion * sc.t0;
				draw_shape(s1xx, RED.opaqued(0.5f));

				s1xx = s1x;
				for (auto& i : s1xx) i += scp2d.motion * sc.t1;
				draw_shape(s1xx, BLUE.opaqued(0.5f));

				out_graph.draw_point(sc.c0, RED);
				out_graph.draw_point(sc.c1, BLUE);
			}
		}

	private:

		real _shapeSize = 2.0f;
		s32 _fillSize = 32;

		Shape _shape1;
		Shape _shape2;
	};
}

#endif