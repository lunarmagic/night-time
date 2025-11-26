#pragma once

#if 0
#include "node/NodeWindow.h"
#include "math/Math.h"
#include "random/random.h"
#include "geometry/Polygon.h"

namespace night
{
	struct TestConvex2D : public NodeWindow
	{
		TestConvex2D(NodeWindowParams nwp = {})
			: NodeWindow(nwp)
		{
			_renderTarget = create<NodeRenderTarget>("Temp FrameBuffer", NodeRenderTargetParams{
				 .depth = 0,
				 .clear_color = LIGHT,
				 .should_use_depth_peeling = false,
				 .should_use_depth_testing = false ,
				 .should_use_blending = false,
				 .should_automatically_clear = true,
				 .should_automatically_render = true,
				});

			ASSERT(_renderTarget != nullptr);

			Camera camera;
			camera.translation = FORWARD * 10.0f;
			camera.look_at = ORIGIN;
			camera.up = UP;
			camera.ortho_region = AABB{ .left = -1, .right = 1, .top = 1, .bottom = -1 };
			camera.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
			camera.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;
			_renderTarget->camera(camera);
		}

	protected:

		virtual void on_update(real delta)
		{
			__super::on_update(delta);
			IGui& gui = utility::gui();

			b8 is_open = true;
			gui.begin("Test Node Animation", &is_open);

			if (!is_open)
			{
				emit_signal("Night Test Closed");
				destroy();
				gui.end();
				return;
			}

			gui.drag_s32("Size", &_fillSize, 0.1f, 0, 64);
			gui.checkbox("Show Index Colors", &_shouldShowIndexColors);

			if (gui.button("Fill With Random Points"))
			{
				_points.clear();
				for (s32 i = 0; i < _fillSize; i++)
				{
					real t = (real)i / (real)(_fillSize - 1);
					_points.push_back(Point{ .point = vec2{random(1.0f) - 0.5f, random(1.0f) - 0.5f }, .index_color = Color::lerp(GREEN, PURPLE, t) });
				}
			}

			if (gui.button("Make Clockwise (concave)"))
			{
				convex2d::make_clockwise(_points.begin(), _points.end());
			}

			gui.end();
		}

		virtual void on_render(RenderGraph& outgraph) const
		{
			for (s32 i = 0; i < _points.size(); i++)
			{
				real t = (real)i / (real)(_points.size() - 1);
				outgraph.draw_point(_points[i].point, _shouldShowIndexColors ? _points[i].index_color : Color::lerp(BLUE, RED, t));
			}
		}

	private:

		handle<NodeRenderTarget> _renderTarget = nullptr;

		struct Point
		{
			vec2 point;
			Color index_color;
			inline operator vec2& () { return point; }
		};

		s32 _fillSize = 32;
		b8 _shouldShowIndexColors = false;
		vector<Point> _points;
	};
}
#endif