
#include "nightpch.h"
#include "ExampleSelect.h"
#include "examples/ExampleBase.h"
#include "examples/ExampleNodeAnimation.h"
#include "examples/ExampleConvex2D.h"
#include "examples/ExampleShapeCast3D.h"
#include "profiler/Profiler.h"

// TODO: add example for intersection between shapes
// TODO: add example for raycasting
// TODO: add example for curves
// TODO: add example for gjk intersects 2d and 3d
// TODO: add example for rendering.

namespace night
{

	ExampleSelect::ExampleSelect()
		: NodeWindow(NodeWindowParams{
				.state = ENodeWindowState::Fullscreen,
				.dock_where = ENodeWindowDockWhere::Centered,
				.dock_space = { 0.7f, 0.0f, 0.0f, 0.0f }
			})
	{
		NodeWindowParams vp_params;
		vp_params.dock_where = ENodeWindowDockWhere::Centered;
		_viewport = create<NodeWindow>("Viewport", vp_params);

		NodeWindowParams gui_params;
		gui_params.dock_where = ENodeWindowDockWhere::Left;
		_gui = create<NodeGui>("Example Select", gui_params);
		_gui->on_close(nullptr);

		NodeRenderTargetParams nrt_params;
		nrt_params.depth = 0;
		nrt_params.clear_color = LIGHT;
		nrt_params.should_use_depth_peeling = false;
		nrt_params.should_use_depth_testing = true;
		nrt_params.should_use_blending = false;
		nrt_params.should_automatically_clear = true;
		nrt_params.should_automatically_render = true;

		nrt_params.camera.type = ECameraType::Orthographic;
		nrt_params.camera.translation = FORWARD * 10.0f;
		nrt_params.camera.look_at = ORIGIN;
		nrt_params.camera.up = UP;
		nrt_params.camera.ortho_region = AABB<>{ .left = -4, .right = 4, .top = 4, .bottom = -4 };
		nrt_params.camera.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
		nrt_params.camera.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;

		_exampleRenderTarget = _viewport->create<NodeRenderTarget>("Example Render Target", nrt_params);
	
		clear_color = LIGHT;
	}

	template<typename T>
	inline void ExampleSelect::create_example()
	{
		ASSERT(_activeExample == nullptr);
		_activeExample = _exampleRenderTarget->create<T>("Active Example");

		_gui->on_close([self = (handle<ExampleSelect>)handle_from_this()]()
			{
				ASSERT(self != nullptr);
				if (self != nullptr)
				{
					if (self->_activeExample != nullptr)
					{
						self->_activeExample->destroy();
						self->_activeExample = nullptr;
						self->_gui->on_close(nullptr);
					}
				}
			});
	}

	void ExampleSelect::on_update(real delta)
	{
		__super::on_update(delta);

		NIGHT_PROFILER_SCOPED("ExampleSelect::on_update");
		ASSERT(_gui != nullptr);
		NODE_GUI_TEMP_BEGIN(_gui);

		if (_activeExample != nullptr)
		{
			_activeExample->update_gui(_gui);
		}
		else
		{
			if (_gui->button("NodeAnimation"))
			{
				create_example<ExampleNodeAnimation>();
			}
			else if (_gui->button("ExampleConvex2D"))
			{
				create_example<ExampleConvex2D>();
			}
			else if (_gui->button("ExampleShapecast3D"))
			{
				create_example<ExampleShapecast3D>();
			}
		}

		NODE_GUI_TEMP_END(_gui);
	}

}