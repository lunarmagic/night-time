#pragma once

#include "ExampleBase.h"
#include "node/NodeGui.h"
#include "ExampleShapeRenderer3D.h"

namespace night
{
	struct ExampleNodeWindow : public ExampleBase
	{
		ExampleNodeWindow()
			: ExampleBase(ECameraType::Orthographic)
		{
			NodeWindowParams ds_params;
			ds_params.borderless = true;
			ds_params.state = ENodeWindowState::Fullscreen;
			ds_params.dock_space = AABB<>{.left = 0.5f, .top = 0.5f, .right = 0.5f, .bottom = 0.5f};
			ds_params.nrt_params.depth = EXAMPLE_RENDER_TARGET_DEPTH + 1;

			// window breaks if not an immediate child of it's parent window
			_dockspace = find_parent<NodeWindow>()->create<NodeWindow>("Dock Space", ds_params);

			vec2 window_size = vec2{ 1.2f, 1.0f };

			NodeWindowParams gui_params;
			gui_params.borderless = false;
			gui_params.state = ENodeWindowState::Free;
			gui_params.dock_where = ENodeWindowDockWhere::None;
			gui_params.position = vec2{ -window_size.x / 2 + window_size.x / 2 + 0.01f, window_size.y / 2 };
			gui_params.size = window_size;
			_testGUI = _dockspace->create<NodeGui>("Test Gui", gui_params);

			NodeWindowParams nw_params;
			nw_params.borderless = false;
			nw_params.state = ENodeWindowState::Free;
			nw_params.dock_where = ENodeWindowDockWhere::None;
			nw_params.position = vec2{ -window_size.x / 2 - window_size.x / 2 - 0.01f, window_size.y / 2 };
			nw_params.size = window_size;
			_testWindow = _dockspace->create<NodeWindow>("Test Window", nw_params);

			NodeRenderTargetParams nrt_params;
			nrt_params.depth = 0;
			nrt_params.should_use_depth_peeling = true;
			nrt_params.should_use_depth_testing = true;
			nrt_params.should_use_blending = true;
			nrt_params.should_automatically_clear = true;
			nrt_params.should_automatically_render = true;
			nrt_params.clear_color = LIGHT;
			nrt_params.camera.type = ECameraType::Perspective;
			nrt_params.camera.translation = FORWARD * (real)10;
			nrt_params.camera.look_at = ORIGIN;
			nrt_params.camera.up = UP;
			nrt_params.camera.ortho_region = AABB<>{ .left = -4, .right = 4, .bottom = -4 };
			nrt_params.camera.fov = NIGHT_CAMERA_DEFAULT_FOV;
			nrt_params.camera.near_clip = NIGHT_CAMERA_DEFAULT_NEAR_CLIP;
			nrt_params.camera.far_clip = NIGHT_CAMERA_DEFAULT_FAR_CLIP;
			_testRenderTarget = _testWindow->create<NodeRenderTarget>("Test Render Target", nrt_params);

			_testShapeRenderer = _testRenderTarget->create<ExampleShapeRenderer3D>("Test Shape Renderer");
		}

		~ExampleNodeWindow()
		{
			ASSERT(_dockspace != nullptr);
			_dockspace->destroy();
		}

		virtual void update_gui(handle<NodeGui> gui)
		{
			ASSERT(gui != nullptr);
			ASSERT(_testShapeRenderer != nullptr);
			ASSERT(_testGUI != nullptr);
			ASSERT(_testWindow != nullptr);

			vec2 m = _testWindow->base_render_target_coord_to_local_coord(utility::window().mouse());
			vec2 mm = _testWindow->base_render_target_motion_to_local_motion(utility::window().mouse_motion());

			gui->text("Test Window Mouse Coord: " + to_string(m));
			gui->text("Test Window Motion Coord: " + to_string(mm));
			
			NODE_GUI_TEMP_BEGIN(_testGUI);
			_testShapeRenderer->update_gui(_testGUI);
			NODE_GUI_TEMP_END(_testGUI);
		}

	protected:

		//virtual void on_render(RenderGraph& out_graph) const override
		//{
		//	
		//}

	private:

		handle<NodeWindow> _dockspace = nullptr;
		handle<NodeGui> _testGUI = nullptr;
		handle<NodeWindow> _testWindow = nullptr;
		handle<NodeRenderTarget> _testRenderTarget = nullptr;
		handle<ExampleShapeRenderer3D> _testShapeRenderer = nullptr;
	};
}