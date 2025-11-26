
#include "nightpch.h"
#include "NodeWindow.h"
#include "texture/ITexture.h"
#include "raycast/Raycast.h"
#include "NodeRenderTarget.h"
#include "event/ApplicationEvent.h"
#include "event/NodeEvent.h"
#include "event/MouseEvent.h"
#include "event/PenEvent.h"
#include "debug_renderer/DebugRenderer.h"

// TODO: impl window_coord_to_local_coord and motion.
// 
// TODO: add resizing.
// 
// TODO: each window's sub-windows should have their
// own window stack.
// 
// TODO: add mem var if we should ignore input events
// when clicked out of the window.
//
// TODO: keep windows within their parent window's
// view area so we can always click and drag the handle bar.
//
// TODO: figure out pen events
// 
// TODO: add a window in 3D space via render target,
// mouse-pick into the 3D quad and interact with that window
//
// TODO: figure out dock resizing and how we handle moving
// windows into docks
//
// TODO: add anti-aliasing setting to DrawLineParams, 
// debug rendering should always be un-anti-aliased

namespace night
{

	list<handle<NodeWindow>> NodeWindow::_windowStack;

	NodeWindow::NodeWindow(NodeWindowParams params)
	{
		_position = params.position;
		_size = params.size;
		_borderless = params.borderless;
		_state = params.state;
		_dockWhere = params.dock_where;
		_dockSpace = params.dock_space;
		_internalResolutionScale = params.internal_resolution_scale;
		_onClose = params.on_close;
		
		// TODO: remove nrt_params and use our own variables.
		ivec2 internal_resolution = calculate_internal_resolution();
		params.nrt_params.width = internal_resolution.x;
		params.nrt_params.height = internal_resolution.y;
		params.nrt_params.filtering = ETextureFiltering::Linear;

		params.nrt_params.should_inherit_parent_resolution = false; // disable before initializing
		NodeRenderTarget::init(params.nrt_params);

		// need to re-enable this after initializing
		should_inherit_parent_resolution = true;

		// TODO: fix unbind event function.
		unbind_all_events();

		_stackIterator = _windowStack.insert(_windowStack.end(), handle_from_this());
		TRACE("created NodeWindow, name: {0}, stack depth: {1}", name_and_id(), _windowStack.size());
	}

	NodeWindow::~NodeWindow()
	{
		ASSERT(_stackIterator != _windowStack.end());
		_windowStack.erase(_stackIterator);
		TRACE("destroyed NodeWindow, name: {0}, stack depth: {1}", name_and_id(), _windowStack.size());
	}

	AABB<> NodeWindow::base_render_target_window_area_rec(AABB<> const& area)
	{
		// this function projects our view area up to the base render target
		// of the renderer, useful for debug rendering
		auto target = this->target();
		ASSERT(target != nullptr);

		AABB<> normalized;
		normalized.top_left(target->global_to_local(area.top_left()));
		normalized.bottom_right(target->global_to_local(area.bottom_right()));

		normalized.left = (normalized.left + 1) / 2;
		normalized.top = (normalized.top + 1) / 2;
		normalized.right = (normalized.right + 1) / 2;
		normalized.bottom = (normalized.bottom + 1) / 2;

		AABB<> area_clamped = window_area_clamped_to_pixel_grid(false);;
		AABB<> result;

		result.left = math::lerp(area_clamped.left, area_clamped.right, normalized.left);
		result.bottom = math::lerp(area_clamped.bottom, area_clamped.top, normalized.bottom);
		result.right = math::lerp(area_clamped.left, area_clamped.right, normalized.right);
		result.top = math::lerp(area_clamped.bottom, area_clamped.top, normalized.top);

		auto parent_window = find_parent<NodeWindow>();
		if (parent_window != nullptr)
		{
			return parent_window->base_render_target_window_area_rec(result);
		}

		return result; // TODO: may break if root window is not fullscreen
	}

	AABB<> NodeWindow::base_render_target_window_area()
	{
		AABB<> ac = window_area_clamped_to_pixel_grid(false);
		auto parent_window = find_parent<NodeWindow>();
		if (parent_window != nullptr)
		{
			return parent_window->base_render_target_window_area_rec(ac);
		}

		return ac; // TODO: don't know if this works
	}

	void NodeWindow::dock_where(ENodeWindowDockWhere where)
	{
		// change where this window is docked in the parent window's dock space.
		if (where != _dockWhere)
		{
			_dockWhere = where;
			ivec2 internal_resolution = calculate_internal_resolution();
			resize(internal_resolution);
		}
	}

	void NodeWindow::dock_space(AABB<> space)
	{
		// set the dock space of this window,
		// then update all sub-windows that are within.
		// TODO: may wan't to only resize children of which docks have actually changed.
		_dockSpace = space;
		dispatch_system([&](NodeWindow& child_window)
			{
				if (child_window.dock_where() == ENodeWindowDockWhere::None)
				{
					return;
				}

				ivec2 internal_resolution = child_window.calculate_internal_resolution();
				child_window.resize(internal_resolution);
			}, stopper<NodeWindow, NodeRenderTarget>);
	}

	void NodeWindow::internal_resolution_scale(real scale)
	{
		_internalResolutionScale = scale;

		ivec2 internal_resolution = calculate_internal_resolution();
		handle_resize(internal_resolution.x, internal_resolution.y);
		WindowResizeEvent e(internal_resolution.x, internal_resolution.y);
		pass_down_event(e);
	}

	ivec2 NodeWindow::calculate_internal_resolution() const
	{
		AABB<s32> internal = window_area_internal();
		ivec2 result;
		result.x = internal.right - internal.left;
		result.y = internal.top - internal.bottom;

		result.x = (s32)((real)result.x * _internalResolutionScale);
		result.y = (s32)((real)result.y * _internalResolutionScale);

		ASSERT(result.x > 0 && result.y > 0);
		return result;
	}

	AABB<> NodeWindow::dock_area(ENodeWindowDockWhere where) const
	{
		ASSERT(target() != nullptr);
		AABB<> area = target()->area();

		switch (where)
		{
		case ENodeWindowDockWhere::Left:
		{
			area.right = area.left + _dockSpace.left;
			break;
		}

		case ENodeWindowDockWhere::Right:
		{
			area.left = area.right - _dockSpace.right;
			break;
		}

		case ENodeWindowDockWhere::Top:
		{
			area.bottom = area.top - _dockSpace.top;
			break;
		}

		case ENodeWindowDockWhere::Bottom:
		{
			area.top = area.bottom + _dockSpace.bottom;
			break;
		}

		case ENodeWindowDockWhere::Centered:
		{
			area.right = area.right - _dockSpace.right;
			area.left = area.left + _dockSpace.left;
			area.top = area.top - _dockSpace.top;
			area.bottom = area.bottom + _dockSpace.bottom;
			break;
		}

		case ENodeWindowDockWhere::None:
		{
			break;
		}
		}

		return area;
	}

	AABB<> NodeWindow::window_area(b8 local) const
	{
		AABB<> result;

		if (_dockWhere != ENodeWindowDockWhere::None)
		{
			auto parent_window = find_parent<NodeWindow>();
			if (parent_window != nullptr)
			{
				auto prt = parent_window->target();
				ASSERT(prt != nullptr);
				result = parent_window->dock_area(_dockWhere);
			}
		}
		else if (_state == ENodeWindowState::Free)
		{
			auto crt = current_render_target();
			ASSERT(crt != nullptr);
			result.left = _position.x;
			result.top = _position.y;
			result.right = _position.x + _size.x;
			result.bottom = _position.y - _size.y;
		}
		else if (_state == ENodeWindowState::Fullscreen)
		{
			auto crt = current_render_target();
			ASSERT(crt != nullptr);
			result = crt->area();
		}

		if (local)
		{
			auto crt = current_render_target();
			ASSERT(crt != nullptr);
			result.top_left(crt->global_to_local(result.top_left()));
			result.bottom_right(crt->global_to_local(result.bottom_right()));
		}

		return result;
	}

	AABB<> NodeWindow::handle_area(b8 local) const
	{
		ASSERT(is_moveable());

		AABB<> wa = window_area(false);
		AABB<> result;

		result.left = wa.left;
		result.top = wa.top + _handleBarHeight;
		result.right = wa.right;
		result.bottom = wa.top;

		if (local)
		{
			auto crt = current_render_target();
			ASSERT(crt != nullptr);
			result.top_left(crt->global_to_local(result.top_left()));
			result.bottom_right(crt->global_to_local(result.bottom_right()));
		}

		return result;
	}

	AABB<s32> NodeWindow::window_area_internal() const
	{
		AABB<> wa = window_area(false);
		AABB<s32> result;
		auto crt = current_render_target();
		ASSERT(crt != nullptr);

		result.top_left(crt->global_to_internal(wa.top_left()));
		result.bottom_right(crt->global_to_internal(wa.bottom_right()));

		return result;
	}

	// TODO areas do not match up for windows that are the same size as there parents
	AABB<> NodeWindow::window_area_clamped_to_pixel_grid(b8 local) const
	{
		AABB<> result;
		auto crt = current_render_target();
		ASSERT(crt != nullptr);

		AABB<s32> internal = window_area_internal();

		if (local)
		{
			result.top_left(crt->internal_to_local(internal.top_left()));
			result.bottom_right(crt->internal_to_local(internal.bottom_right()));
		}
		else
		{
			result.top_left(crt->internal_to_global(internal.top_left()));
			result.bottom_right(crt->internal_to_global(internal.bottom_right()));
		}

		return result;
	}

	void NodeWindow::on_render(RenderGraph& out_graph) const
	{
		u64 depth = window_stack_depth();
		real t = (real)depth / (real)_windowStack.size();
		// TODO: replace 50.0f magic number
		real window_depth = math::lerp(50.0f, 99.0f, t);

		Color border_color = _borderColor;
		Color outline_color = _outlineColor;

		if (is_at_top_of_window_stack())
		{
			border_color = border_color.darken(0.5f); // TODO: selected color
			outline_color = outline_color.darken(0.5f);
		}

		// get area of window render target, not with borders:
		AABB<> actpg = window_area_clamped_to_pixel_grid(false);
		if (actpg.left >= actpg.right && actpg.bottom >= actpg.top)
		{
			// window is closed.
			return;
		}

		{
			// draw our render target to the parent render target.
			Quad qarea(actpg);

			for (s32 j = 0; j < qarea.vertices.size(); j++)
			{
				qarea.vertices[j].point.z = window_depth;
			}

			out_graph.draw_quad(qarea);
		}

		{
			// handle border:
			if (is_moveable())
			{
				// do not render with our render target's texture.
				out_graph.current_buffer(out_graph.current_render_target(), nullptr, nullptr);

				vec2 top_right = vec2{ actpg.right + _outerEdgeWidth, actpg.top + _handleBarHeight + _outerEdgeWidth };
				vec2 bottom_right = vec2{ actpg.right + _outerEdgeWidth, actpg.bottom - _outerEdgeWidth };
				vec2 bottom_left = vec2{ actpg.left - _outerEdgeWidth, actpg.bottom - _outerEdgeWidth };
				vec2 top_left = vec2{ actpg.left - _outerEdgeWidth, actpg.top + _handleBarHeight + _outerEdgeWidth };

				Triangle t1;
				t1.vertices[0].point = vec4{ top_right, window_depth - NIGHT_EPSILON_MEDIUM, 1.0f };
				t1.vertices[1].point = vec4{ bottom_right, window_depth - NIGHT_EPSILON_MEDIUM, 1.0f };
				t1.vertices[2].point = vec4{ bottom_left, window_depth - NIGHT_EPSILON_MEDIUM, 1.0f };
				t1.vertices[0].color = border_color;
				t1.vertices[1].color = border_color;
				t1.vertices[2].color = border_color;
				out_graph.draw_triangle(t1);

				Triangle t2;
				t2.vertices[0].point = vec4{ bottom_left, window_depth - NIGHT_EPSILON_MEDIUM, 1.0f };
				t2.vertices[1].point = vec4{ top_left, window_depth - NIGHT_EPSILON_MEDIUM, 1.0f };
				t2.vertices[2].point = vec4{ top_right, window_depth - NIGHT_EPSILON_MEDIUM, 1.0f };
				t2.vertices[0].color = border_color;
				t2.vertices[1].color = border_color;
				t2.vertices[2].color = border_color;
				out_graph.draw_triangle(t2);

				if (_edgeSmoothing != 0.0f)
				{
					out_graph.draw_line(vec3{ top_left, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, vec3{ top_right, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, outline_color, _edgeSmoothing);
					out_graph.draw_line(vec3{ top_right, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, vec3{ bottom_right, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, outline_color, _edgeSmoothing);
					out_graph.draw_line(vec3{ bottom_right, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, vec3{ bottom_left, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, outline_color, _edgeSmoothing);
					out_graph.draw_line(vec3{ bottom_left, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, vec3{ top_left, window_depth - NIGHT_EPSILON_MEDIUM / 2 }, outline_color, _edgeSmoothing);
				}
			}
		}

		{
			// draw child non-window render targets to our render target:
			multimap<real, NodeRenderTarget&> rt_sorted;

			dispatch_system([&](NodeRenderTarget& node)
				{
					if (node.is_of_type<NodeWindow>())
					{
						return;
					}

					if (node.visibility != EVisibility::Visible)
					{
						return;
					}

					rt_sorted.insert({ node.depth, node });
				}, stopper<NodeWindow, NodeRenderTarget>);


			auto target = this->target();
			ASSERT(target != nullptr);
			Quad rt_quad(target->area());

			for (const auto& i : rt_sorted)
			{
				out_graph.current_buffer(
					target,
					i.second.material,
					i.second.textures()
				);

				Quad quad = rt_quad;

				for (s32 j = 0; j < quad.vertices.size(); j++)
				{
					quad.vertices[j].point.z = quad.vertices[j].point.z + i.first;
				}

				out_graph.draw_quad(quad);
			}
		}
	}

	vec2 NodeWindow::mouse()
	{
		return base_render_target_coord_to_local_coord(utility::window().mouse());
	}

	Ray3D<> NodeWindow::mouse_pick(vec2 const& window_mouse_position) const
	{
		ASSERT(_target != nullptr);
		vec2 local_coord = base_render_target_coord_to_local_coord(window_mouse_position);
		return _target->mouse_pick(local_coord);
	}

	b8 NodeWindow::is_moveable() const
	{
		return (!_borderless && _state != ENodeWindowState::Fullscreen && _dockWhere == ENodeWindowDockWhere::None);
	}

	void NodeWindow::position(vec2 const& position)
	{
		_position = position;
		// TODO: keep window within bounds
	}

	// TODO: rename
	vec2 NodeWindow::passed_down_local_coord(vec2 const& parent_mouse_position) const
	{
		// this function converts from parent local space to our local space
		auto crt = this->current_render_target();
		ASSERT(crt != nullptr);
		AABB<> ac = window_area_clamped_to_pixel_grid(false);
		vec2 local = ac.local_coordinate(crt->local_to_global(parent_mouse_position));
		return local;
	}

	// TODO: rename
	vec2 NodeWindow::passed_down_local_motion(vec2 const& parent_mouse_motion) const
	{
		// this function converts from parent local space to our local space
		AABB<> ac = window_area_clamped_to_pixel_grid(false);
		vec2 m = (parent_mouse_motion / vec2(ac.width(), ac.height())) * (real)2;
		auto crt = current_render_target();
		ASSERT(crt != nullptr);
		return crt->local_to_global(m);
	}

	vec2 NodeWindow::base_render_target_coord_to_local_coord(vec2 const& window_coord) const
	{
		auto parent = find_parent<NodeWindow>();
		if (parent == nullptr)
		{
			// we are at the base window
			return passed_down_local_coord(window_coord);
		}

		// TODO: handle when the window is under a NodeRenderTarget

		return passed_down_local_coord(parent->base_render_target_coord_to_local_coord(window_coord));
	}

	vec2 NodeWindow::base_render_target_motion_to_local_motion(vec2 const& window_motion) const
	{
		auto parent = find_parent<NodeWindow>();
		if (parent == nullptr)
		{
			// we are at the base window
			return passed_down_local_motion(window_motion);
		}

		return passed_down_local_motion(parent->base_render_target_motion_to_local_motion(window_motion));
	}

	void NodeWindow::on_event(Event& event, b8 pass_down)
	{
		switch (event.type())
		{
		case EEventType::MouseButtonPressed:
		{
			__super::on_event(event, false);

			vec2 parent_mouse_position = ((MouseButtonPressedEvent&)event).position();
			AABB<> window_area = this->window_area_clamped_to_pixel_grid(true);

			AABB<> total_area = window_area;

			if (is_moveable())
			{
				total_area = AABB<>::combine(total_area, this->handle_area(true));
			}

			if (!total_area.contains(parent_mouse_position))
			{
				// mouse must be within window to consider mouse pressed events
				break;
			}

			// handle window moving and resizing:
			vec2 pass_down_position = passed_down_local_coord(parent_mouse_position);

			if (_grabbedWindow == nullptr)
			{
				handle<NodeWindow> max_window = nullptr;
				AABB<> max_window_area = {};
				AABB<> max_handle_area = {};
				u64 max_depth = numeric_limits<u64>::lowest();

				dispatch_system([&](NodeWindow& window)
					{
						u64 depth = window.window_stack_depth();
						if (depth < max_depth) return;
						if (!window.is_moveable()) return;

						AABB<> wa = window.window_area_clamped_to_pixel_grid(true);
						AABB<> ha = window.handle_area(true);
						AABB<> area = AABB<>::combine(wa, ha);

						if (!area.contains(pass_down_position)) return;

						max_window = window.handle_from_this();
						max_window_area = wa;
						max_handle_area = ha;
						max_depth = depth;
					}, stopper<NodeWindow>);

				if (max_window == nullptr)
				{
					_grabbedWindowAction = EGrabbedNodeWindowAction::None;
					_grabbedWindow = nullptr;
				}
				else
				{
					max_window->push_to_top_of_window_stack();

					if (pass_down_position.y > max_window_area.top)
					{
						// clicked on handle:
						_grabbedWindow = max_window;
						_grabbedWindowAction = EGrabbedNodeWindowAction::Move;
					}
					else
					{
						// clicked inside window:
						_grabbedWindowAction = EGrabbedNodeWindowAction::None;
					}
				}
			}

			if (window_area.contains(parent_mouse_position))
			{
				MouseButtonPressedEvent e(((MouseButtonPressedEvent&)event).button(), pass_down_position);
				pass_down_event(e);
			}

			break;
		}

		case EEventType::MouseButtonReleased:
		{
			__super::on_event(event, false);

			if (_grabbedWindow != nullptr)
			{
				_grabbedWindow = nullptr;
				_grabbedWindowAction = EGrabbedNodeWindowAction::None;
			}

			MouseButtonReleasedEvent e(((MouseButtonPressedEvent&)event).button(), passed_down_local_coord(((MouseButtonReleasedEvent&)event).position()));
			pass_down_event(e);
			break;
		}

		case EEventType::MouseMotion:
		{
			__super::on_event(event, false);

			vec2 pass_down_motion = passed_down_local_motion(((MouseMotionEvent&)event).motion());
			RenderTarget crt = current_render_target();
			ASSERT(crt != nullptr);

			if (_grabbedWindow != nullptr)
			{
				if (_grabbedWindowAction == EGrabbedNodeWindowAction::Move)
				{
					_grabbedWindow->position(_grabbedWindow->position() + crt->local_to_global(pass_down_motion));
				}
			}

			MouseMotionEvent e(pass_down_motion);
			pass_down_event(e);
			break;
		}

		case EEventType::PenDown:
		{
			__super::on_event(event, false);
			PenDownEvent e(passed_down_local_coord(((PenDownEvent&)event).position()), ((PenDownEvent&)event).is_eraser(), ((PenDownEvent&)event).id());
			pass_down_event(e);
			break;
		}

		case EEventType::PenUp:
		{
			__super::on_event(event, false);
			PenUpEvent e(passed_down_local_coord(((PenUpEvent&)event).position()), ((PenUpEvent&)event).is_eraser(), ((PenUpEvent&)event).id());
			pass_down_event(e);
			break;
		}

		case EEventType::PenMotion:
		{
			__super::on_event(event, false);
			PenMotionEvent e(passed_down_local_coord(((PenMotionEvent&)event).position()), ((PenMotionEvent&)event).id());
			pass_down_event(e);
			break;
		}

		case EEventType::WindowResize:
		{
			__super::on_event(event, false);
			ivec2 internal_resolution = calculate_internal_resolution();
			handle_resize(internal_resolution.x, internal_resolution.y);
			WindowResizeEvent e(internal_resolution.x, internal_resolution.y);
			pass_down_event(e);
			break;
		}

		case EEventType::NodeMoved:
		{
			__super::on_event(event, false);
			ASSERT(_target != nullptr);
			_target->render_flush_priority((real)-depth_from_root()); // TODO: make sure this works.
			ivec2 internal_resolution = calculate_internal_resolution();
			handle_resize(internal_resolution.x, internal_resolution.y);
			WindowResizeEvent e(internal_resolution.x, internal_resolution.y);
			pass_down_event(e);
			break;
		}

		default:
		{
			__super::on_event(event, true);
			break;
		}
		}
	}

	u64 NodeWindow::window_stack_depth() const
	{
		ASSERT(_stackIterator != _windowStack.end());
		return (u64)std::distance(_windowStack.begin(), _stackIterator);
	}

	void NodeWindow::push_to_top_of_window_stack()
	{
		ASSERT(_stackIterator != _windowStack.end());
		_windowStack.erase(_stackIterator);
		_stackIterator = _windowStack.insert(_windowStack.end(), handle_from_this());
		TRACE("NodeWindow pushing to top of window stack, name: {0}, stack size: {1}", name_and_id(), _windowStack.size());
	}

	b8 NodeWindow::is_at_top_of_window_stack() const
	{
		return _stackIterator == std::prev(_windowStack.end());
	}

}