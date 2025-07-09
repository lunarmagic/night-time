#pragma once

//#include "NodeRenderable.h"
#include "NodeRenderTarget.h"
//#include "debug_renderer/DebugRenderer.h"

namespace night
{

	enum struct ENodeWindowState : u32
	{
		Free = 0,
		Expanded,
		Minimized,
		Fullscreen,
		//Docked,
		OutsideMainWindow
	};

	enum struct ENodeWindowDockWhere : u32
	{
		None = 0,
		Centered,
		Left,
		Right,
		Top,
		Bottom
		// TODO make more dockspaces for in between the above ones
	};

	struct NodeWindowParams
	{
		//AABB area{ .left = INFINITY, .right = -INFINITY, .top = -INFINITY, .bottom = INFINITY };
		vec2 position{vec2(-1.0f, 1.0f)}; // top left of window
		vec2 size{ vec2(2.0f) };
		u8 borderless = false;
		ENodeWindowState state{ENodeWindowState::Free};
		ENodeWindowDockWhere dock_where{ ENodeWindowDockWhere::None };
		AABB dock_space{ AABB{ 0, 0, 0, 0 } };

		//Color clear_color{ LIGHT };
		NodeRenderTargetParams nrt_params;
	};

	struct NIGHT_API NodeWindow : public NodeRenderTarget
	{
		NodeWindow(NodeWindowParams params = {});

		vec2 window_coord_to_local_coord(vec2 const& window_coord) const;
		vec2 window_motion_to_local_motion(vec2 const& window_motion) const;

		Ray mouse_pick(vec2 const& window_mouse_position) const;

		Quad global_area();

		void dock_where(ENodeWindowDockWhere where);
		ENodeWindowDockWhere const& dock_where() const { return _dockWhere; }

		void dock_space(AABB space);
		AABB const& dock_space() const { return _dockSpace; }

		//ivec2 dock_space_resolution(ENodeWindowDockWhere where);

		ivec2 find_internal_resolution() const;

	protected:

		virtual void on_render(RenderGraph& out_graph) const  override;

	private:

		vec2 event_mouse_position(vec2 const& parent_mouse_position) const;
		vec2 event_mouse_motion(vec2 const& parent_mouse_motion) const;

		// depth of most recently clicked/created window is set to max
		// TODO: make static depth for all windows, increment by one every click/creation
		vec2 _position{};
		vec2 _size{};
		u8 _borderless;
		ENodeWindowState _state;
		u64 _windowDepth{ 0 };
		AABB _dockSpace{ AABB{ 0, 0, 0, 0 } };
		ENodeWindowDockWhere _dockWhere{ ENodeWindowDockWhere::None };

		ivec4 area_internal() const;
		AABB area_clamped_to_pixel_grid() const;
		//AABB clamp_area_to_pixel_grid(AABB const& area) const;
		AABB docking_area(ENodeWindowDockWhere where) const;
		//Quad _area;

		Quad global_area_rec(AABB area);

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		friend struct DebugRenderer;
#endif
	};


}