#pragma once

#include "ExampleBase.h"
#include "random/random.h"
#include "geometry/Polygon.h"

namespace night
{
	struct ExampleConvex2D : public ExampleBase
	{

	protected:

		virtual void update_gui(handle<NodeGui> gui) override
		{
			ASSERT(gui != nullptr);

			gui->drag_s32("Size", &_fillSize, 0.1f, 0, 64);
			gui->checkbox("Show Index Colors", &_shouldShowIndexColors);

			if (gui->button("Fill With Random Points"))
			{
				_points.clear();
				for (s32 i = 0; i < _fillSize; i++)
				{
					real t = (real)i / (real)(_fillSize - 1);
					_points.push_back(Point{ .point = vec2{ random(5.0f) - 2.5f, random(5.0f) - 2.5f }, .index_color = Color::lerp(GREEN, PURPLE, t) });
				}
			}

			if (gui->button("Make Clockwise (concave)"))
			{
				Convex2D<>::make_clockwise(_points.begin(), _points.end());
			}

			// TODO: add make convex, is intersecting mouse, is clockwise, raycast,
			// area and clip.
		}

		virtual void on_render(RenderGraph& out_graph) const override
		{
			for (s32 i = 0; i < _points.size(); i++)
			{
				real t = (real)i / (real)(_points.size() - 1);
				out_graph.draw_point(_points[i].point, _shouldShowIndexColors ? _points[i].index_color : Color::lerp(BLUE, RED, t));
			}
		}

	private:

		struct Point
		{
			vec2 point;
			Color index_color;
			inline operator vec2& () { return point; }
		};

		s32 _fillSize = 32;
		u8 _shouldShowIndexColors = false;
		vector<Point> _points;
	};
}