
#include "nightpch.h"
#include "NodeWindow.h"
#include "texture/ITexture.h"
#include "raycast/raycast.h"
#include "NodeRenderTarget.h"
#include "event/ApplicationEvent.h"
#include "event/NodeEvent.h"
#include "event/MouseEvent.h"
#include "event/PenEvent.h"
#include "debug_renderer/DebugRenderer.h"

namespace night
{

	NodeWindow::NodeWindow(NodeWindowParams params)
	{
		_position = params.position;
		_size = params.size;
		_borderless = params.borderless;
		_state = params.state;
		_dockWhere = params.dock_where;
		_dockSpace = params.dock_space;
		_internalResolutionScale = params.internal_resolution_scale;
		
		// TODO: remove nrt_params and use our own variables.
		ivec2 internal_resolution = find_internal_resolution();
		params.nrt_params.width = internal_resolution.x;
		params.nrt_params.height = internal_resolution.y;
		params.nrt_params.filtering = ETextureFiltering::Linear;
		params.nrt_params.should_inherit_parent_resolution = false; // disable before initializing
		NodeRenderTarget::init(params.nrt_params);

		// need to enable this after initializing
		should_inherit_parent_resolution = true;

		unpass_down_event_type(MouseButtonPressedEvent::get_static_type());
		unpass_down_event_type(MouseButtonReleasedEvent::get_static_type());
		unpass_down_event_type(MouseMotionEvent::get_static_type());

		unpass_down_event_type(PenDownEvent::get_static_type());
		unpass_down_event_type(PenUpEvent::get_static_type());
		unpass_down_event_type(PenMotionEvent::get_static_type());

		unpass_down_event_type(WindowResizeEvent::get_static_type());

		// TODO: fix unbind event function.
		unbind_all_events();

		bind_event([&](MouseButtonPressedEvent const& event)
			{
				MouseButtonPressedEvent e(event.button(), event_mouse_position(event.position()));
				pass_down_event(e);
			});

		bind_event([&](MouseButtonReleasedEvent const& event)
			{
				MouseButtonReleasedEvent e(event.button(), event_mouse_position(event.position()));
				pass_down_event(e);
			});

		bind_event([&](MouseMotionEvent const& event)
			{
				MouseMotionEvent e(event_mouse_motion(event.motion()));
				pass_down_event(e);
			});

		bind_event([&](PenDownEvent const& event)
			{
				PenDownEvent e(event_mouse_position(event.position()), event.is_eraser(), event.id());
				pass_down_event(e);
			});

		bind_event([&](PenUpEvent const& event)
			{
				PenUpEvent e(event_mouse_position(event.position()), event.is_eraser(), event.id());
				pass_down_event(e);
			});

		bind_event([&](PenMotionEvent const& event)
			{
				PenMotionEvent e(event_mouse_motion(event.position()), event.id());
				pass_down_event(e);
			});

		bind_event([&](WindowResizeEvent const& event)
		{
			ivec2 internal_resolution = find_internal_resolution();
			handle_resize(internal_resolution.x, internal_resolution.y);
			WindowResizeEvent e(internal_resolution.x, internal_resolution.y);
			pass_down_event(e);
		});

		bind_event([&](NodeMovedEvent const& event)
		{
			ASSERT(_target != nullptr);
			_target->render_flush_priority((real)-depth_from_root()); // TODO: make sure this works.

			ivec2 internal_resolution = find_internal_resolution();
			handle_resize(internal_resolution.x, internal_resolution.y);
			WindowResizeEvent e(internal_resolution.x, internal_resolution.y);
			pass_down_event(e);
		});
	}

	vec2 NodeWindow::window_coord_to_local_coord(vec2 const& window_coord) const
	{
		// TODO: impl
		return event_mouse_position(window_coord);
	}

	vec2 NodeWindow::window_motion_to_local_motion(vec2 const& window_motion) const
	{
		// TODO: impl
		return event_mouse_motion(window_motion);
	}

	Ray NodeWindow::mouse_pick(vec2 const& window_mouse_position) const
	{
		ASSERT(_target != nullptr);
		vec2 local_coord = window_coord_to_local_coord(window_mouse_position);
		return _target->mouse_pick(local_coord);
	}

	Quad NodeWindow::global_area()
	{
		AABB ac = area_clamped_to_pixel_grid();
		auto parent_window = find_parent<NodeWindow>();
		if (parent_window != nullptr)
		{
			return parent_window->global_area_rec(ac);
		}
		else
		{
			return Quad(ac); // TODO: don't know if this works
		}
	}

	void NodeWindow::dock_where(ENodeWindowDockWhere where)
	{
		if (where != _dockWhere)
		{
			_dockWhere = where;
			ivec2 internal_resolution = find_internal_resolution();
			resize(internal_resolution);
		}
	}

	void NodeWindow::dock_space(AABB space)
	{
		// TODO: may wan't to only resize children of which docks have actually changed.
		_dockSpace = space;
		dispatch_system([&](NodeWindow& child_window)
			{
				if (child_window.dock_where() == ENodeWindowDockWhere::None)
				{
					return;
				}

				ivec2 internal_resolution = child_window.find_internal_resolution();
				child_window.resize(internal_resolution);
			}, exclude<NodeWindow, NodeRenderTarget>);
	}

	Quad NodeWindow::global_area_rec(AABB area)
	{
		auto target = this->target();
		ASSERT(target != nullptr);
		vec3 tlproj = target->project_to_screen({ area.left, area.top, 0 });
		vec3 brproj = target->project_to_screen({ area.right, area.bottom, 0 });

		tlproj.x += 1;
		tlproj.x /= 2;
		brproj.x += 1;
		brproj.x /= 2;

		tlproj.y += 1;
		tlproj.y /= 2;
		brproj.y += 1;
		brproj.y /= 2;

		tlproj.z = 0;
		tlproj.z = 0;
		brproj.z = 0;
		brproj.z = 0;

		AABB ac = area_clamped_to_pixel_grid();

		AABB ap;
		ap.left = lerp(ac.left, ac.right, tlproj.x);
		ap.right = lerp(ac.left, ac.right, brproj.x);
		ap.bottom = lerp(ac.bottom, ac.top, brproj.y);
		ap.top = lerp(ac.bottom, ac.top, tlproj.y);
		

		auto parent_window = find_parent<NodeWindow>();
		if (parent_window != nullptr)
		{
			return parent_window->global_area_rec(ap);
		}
		else
		{
			return Quad(ap); // TODO: may break if root window is not fullscreen
		}
	}

	vec2 NodeWindow::event_mouse_position(vec2 const& parent_mouse_position) const
	{
		auto crt = this->current_render_target();
		ASSERT(crt != nullptr);
		auto target = this->target();
		ASSERT(target != nullptr);
		AABB ac = area_clamped_to_pixel_grid();
		vec2 local = ac.local_coordinate(crt->unproject_from_screen(vec3(parent_mouse_position, 0)));
		return local;
	}

	vec2 NodeWindow::event_mouse_motion(vec2 const& parent_mouse_motion) const
	{
		AABB ac = area_clamped_to_pixel_grid();
		vec2 m = (parent_mouse_motion / vec2(ac.width(), ac.height())) * 2.0f;
		auto crt = current_render_target();
		ASSERT(crt != nullptr);
		return crt->unproject_from_screen(vec3(m, 0));
	}

	ivec2 NodeWindow::find_internal_resolution() const
	{
		ivec4 internal = area_internal();
		ivec2 result;
		result.x = internal.y - internal.x;
		result.y = internal.z - internal.w;

		ASSERT(_internalResolutionScale != 0);
		result.x = (s32)((real)result.x * _internalResolutionScale);
		result.y = (s32)((real)result.y * _internalResolutionScale);

		return result;
	}

	void NodeWindow::internal_resolution_scale(real scale)
	{
		_internalResolutionScale = scale;

		ivec2 internal_resolution = find_internal_resolution();
		handle_resize(internal_resolution.x, internal_resolution.y);
		WindowResizeEvent e(internal_resolution.x, internal_resolution.y);
		pass_down_event(e);
	}

	// TODO: make iAABB
	ivec4 NodeWindow::area_internal() const
	{
		if (_dockWhere != ENodeWindowDockWhere::None)
		{
			auto parent_window = find_parent<NodeWindow>();
			if (parent_window != nullptr)
			{
				auto prt = parent_window->target();
				ASSERT(prt != nullptr);
				AABB docked_area = parent_window->docking_area(_dockWhere);

				ivec2 internal_tl = prt->global_to_internal(vec2(docked_area.left, docked_area.top));
				ivec2 internal_br = prt->global_to_internal(vec2(docked_area.right, docked_area.bottom));

				// TODO: maybe remove
				// make sure it is a even number
				internal_tl.x = (internal_tl.x / 2) * 2;
				internal_tl.y = (internal_tl.y / 2) * 2;
				internal_br.x = (internal_br.x / 2) * 2;
				internal_br.y = (internal_br.y / 2) * 2;

				ivec4 result;
				result.x = internal_tl.x;
				result.y = internal_br.x;
				result.z = internal_tl.y;
				result.w = internal_br.y;
				return result;
			}
		}

		auto crt = current_render_target();
		ASSERT(crt != nullptr);

		if (_state == ENodeWindowState::Free)
		{
			ivec2 internal_tl = crt->global_to_internal(vec2(_position.x, _position.y));
			ivec2 internal_br = crt->global_to_internal(vec2(_position.x + _size.x, _position.y - _size.y));

			// TODO: maybe remove
			// make sure it is a even number
			internal_tl.x = (internal_tl.x / 2) * 2;
			internal_tl.y = (internal_tl.y / 2) * 2;
			internal_br.x = (internal_br.x / 2) * 2;
			internal_br.y = (internal_br.y / 2) * 2;

			ivec4 result;
			result.x = internal_tl.x;
			result.y = internal_br.x;
			result.z = internal_tl.y;
			result.w = internal_br.y;
			return result;
		}
		else if (_state == ENodeWindowState::Fullscreen)
		{
			return
			{
				0, // left
				crt->width(), // right
				crt->height(), // top
				0 // bottom
			};
		}

		ASSERT(false); // TODO: handle this case
		return {};
	}

	// TODO areas do not match up for windows that are the same size as there parents
	AABB NodeWindow::area_clamped_to_pixel_grid() const
	{
		AABB result;
		auto crt = current_render_target();
		ASSERT(crt != nullptr);

		ivec4 internal = area_internal();

		vec2 local_tl = crt->internal_to_global(vec2(internal.x, internal.z));
		vec2 local_br = crt->internal_to_global(vec2(internal.y, internal.w));

		result.left = local_tl.x;
		result.right = local_br.x;
		result.top = local_tl.y;
		result.bottom = local_br.y;

		return result;
	}

	AABB NodeWindow::docking_area(ENodeWindowDockWhere where) const
	{
		ASSERT(target() != nullptr);
		AABB area = target()->area();

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

	// TODO: may want parent window to handle child windows rendering,
	// then docking and picking should be easier
	void NodeWindow::on_render(RenderGraph& out_graph) const
	{
		auto target = this->target();
		ASSERT(target != nullptr);

		// draw window render target:
		AABB ac = area_clamped_to_pixel_grid();
		if (ac.left >= ac.right && ac.bottom >= ac.top)
		{
			// window is closed.
			return;
		}

		Quad qarea(ac);
		out_graph.draw_quad(qarea); 

		// draw rts into window render target:
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
		}, exclude<NodeWindow, NodeRenderTarget>);

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