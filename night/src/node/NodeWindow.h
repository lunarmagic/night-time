#pragma once

#include "NodeRenderTarget.h"

namespace night
{

	enum struct ENodeWindowState : u32
	{
		Free = 0,
		Expanded,
		Minimized,
		Fullscreen,
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
	};

	enum struct EGrabbedNodeWindowAction : s32
	{
		None = 0,
		Move,
		Resize
	};

	struct NodeWindowParams
	{
		vec2 position{vec2(-1.0f, 1.0f)}; // top left of window
		vec2 size{ vec2(2.0f) };
		b8 borderless = false;
		ENodeWindowState state{ENodeWindowState::Free};
		ENodeWindowDockWhere dock_where{ ENodeWindowDockWhere::None };
		AABB<> dock_space{ AABB<>{ .left = 0, .top = 0, .right = 0, .bottom = 0 } };

		real internal_resolution_scale{ 1 };

		function<void()> on_close = nullptr;

		NodeRenderTargetParams nrt_params;
	};

	struct NIGHT_API NodeWindow : public NodeRenderTarget
	{
		NodeWindow(NodeWindowParams params = {});
		~NodeWindow();

		AABB<> base_render_target_window_area();

		void dock_where(ENodeWindowDockWhere where);
		ENodeWindowDockWhere const& dock_where() const { return _dockWhere; }

		void dock_space(AABB<> space);
		AABB<> const& dock_space() const { return _dockSpace; }

		void internal_resolution_scale(real scale);
		real const& internal_resolution_scale() const { return _internalResolutionScale; }

		void on_close(function<void()> callback) { _onClose = callback; };
		function<void()> const& on_close() const { return _onClose; }

		vec2 mouse();
		Ray3D<real> mouse_pick(vec2 const& local_coord) const;

		b8 is_moveable() const;

		vec2 const& position() const { return _position; };
		void position(vec2 const& position);

		vec2 base_render_target_coord_to_local_coord(vec2 const& window_coord) const;
		vec2 base_render_target_motion_to_local_motion(vec2 const& window_motion) const;

		virtual void on_event(Event& event, b8 pass_down_event = true) override;

		u64 window_stack_depth() const;
		void push_to_top_of_window_stack();
		b8 is_at_top_of_window_stack() const;

	protected:

		virtual void on_render(RenderGraph& out_graph) const  override;

	private:

		vec2 passed_down_local_coord(vec2 const& local_coord) const;
		vec2 passed_down_local_motion(vec2 const& local_motion) const;

		// TODO: make static depth for all windows, increment by one every click/creation
		vec2 _position{};
		vec2 _size{};
		b8 _borderless;
		ENodeWindowState _state;
		u64 _windowDepth{ 0 };
		AABB<> _dockSpace{ AABB<>{ .left = 0, .top = 0, .right = 0, .bottom = 0 } };
		ENodeWindowDockWhere _dockWhere{ ENodeWindowDockWhere::None };

		Color _borderColor = GREY;
		Color _outlineColor = GREY.darken(0.75f);
		real _handleBarHeight = 0.03f;
		real _outerEdgeWidth = 0.003f;
		real _edgeSmoothing = 0.003f;

		handle<NodeWindow> _grabbedWindow = nullptr;
		EGrabbedNodeWindowAction _grabbedWindowAction = EGrabbedNodeWindowAction::None;

		static list<handle<NodeWindow>> _windowStack;
		list<handle<NodeWindow>>::iterator _stackIterator;

		real _internalResolutionScale{ 1 };

		function<void()> _onClose = nullptr;

		ivec2 calculate_internal_resolution() const;

		AABB<> window_area(b8 local) const;
		AABB<> handle_area(b8 local) const;
		AABB<s32> window_area_internal() const;
		AABB<> window_area_clamped_to_pixel_grid(b8 local) const;
		
		AABB<> dock_area(ENodeWindowDockWhere where) const;

		AABB<> base_render_target_window_area_rec(AABB<> const& area);

		//vec2 window_coord_to_local_coord_rec(vec2 const& window_coord) const;
		//vec2 window_motion_to_local_motion_rec(vec2 const& window_motion) const;
	};

}