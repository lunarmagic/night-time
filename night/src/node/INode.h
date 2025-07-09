#pragma once

#include "core.h"
#include "log/log.h"
#include "event/Event.h"
#include "input/InputKey.h"
#include "math/math.h"
//#include "ref/ref.h"
#include "handle/handle.h"
//#include "stl/stl.h"
#include "debug_renderer/DebugRenderer.h"
#include "event/EventManager.h"

#define BIND_INPUT(key, type, callback) ::night::Application::get().bind_input(key, type, callback) 

namespace night
{
	struct KeyPressedEvent;
	struct KeyReleasedEvent;
	struct MouseButtonPressedEvent;
	struct MouseButtonReleasedEvent;
	struct MouseWheelEvent;
	struct MouseMotionEvent;
	struct PenPressureEvent;
	struct PenDownEvent;
	struct PenUpEvent;
	struct PenMotionEvent;
	struct WindowCloseEvent;
	struct WindowResizeEvent;
	struct RendererPresentedEvent;
	struct NodeMovedEvent;

	template<typename... T>
	struct Exclude
	{
	};

	template<typename... T>
	Exclude<T...> exclude;

	struct NIGHT_API INode
	{
		template<typename T, typename... Args> handle<T> create(const string& name, const Args&... args);
		void remove(const string& name);
		void remove_all();
		handle<INode> find(const string& name) const;

		void destroy();

		void move(handle<INode> new_parent);

		template<typename... Stop>
		void dispatch_system(auto fn, Exclude<Stop...> const& stop, u8 include_newly_created_children = false) const
		{
			dispatch_system_impl(function(fn), stop, include_newly_created_children);
		}

		void dispatch_system(auto fn, u8 include_newly_created_children = false) const
		{
			dispatch_system_impl(function(fn), exclude<>, include_newly_created_children);
		}

		handle<INode> parent() const { return _parent; }
		const vector<shandle<INode>>& children() const { return _children; }

		template<typename T>
		handle<T> find_parent() const;

		const string& name() const { return _name; }
		const u64& unique_id() const { return _unique_id; }

		type_index const& type_id() const { return _type_id; }

		r64 timestamp() const { return _timestamp; }
		void reset_timestamp();
		r64 time_elapsed() const;

		u8 is_active{ true }; // if false, updates and events are disabled for whole branch, but rendering is still enabled.
		u8 is_taking_events{ true };
		real lifespan{ -1.0f };

		u8 is_pending_destruction() const { return _isPendingDestruction; }

		INode();
		virtual ~INode();

		void on_event(Event& event);

		s32 depth_from_root() const;

		// intended for asserts
		template<typename T>
		u8 is_of_type() const
		{
			//return dynamic_cast<T*>(handle_from_this().ptr().lock().get()) != nullptr;
			return dynamic_cast<T*>(const_cast<INode*>(this)) != nullptr;
		}

		handle<const ITexture> current_render_target() const;
		
	protected:

		virtual handle<const ITexture> current_render_target_impl() const;

		static u64 __uid;
		static string __name;
		static handle<INode> __parent;
		static handle<INode> __handle_from_this;
		static type_index __type_id;

		virtual void on_update(real delta) { return; }

		// EVENT MANAGER:
		void bind_input(EKey key, EInputType type, function<void()> fn)
		{
			_eventManager.bind_input(key, type, fn);
		}

		void bind_input(EMouse mouse, EInputType type, function<void()> fn)
		{
			_eventManager.bind_input(mouse, type, fn);
		}

		void bind_input(EButton button, EInputType type, function<void()> fn)
		{
			_eventManager.bind_input(button, type, fn);
		}

		void bind_input(string action, EInputType type, function<void()> fn)
		{
			_eventManager.bind_input(action, type, fn);
		}

		// pass in the iterator returned from bind_input.
		//void unbind_input(umultimap<InputKey, function<void()>>::iterator input)
		//{
		//	_eventManager.unbind_input(input);
		//}

		void unbind_all_inputs()
		{
			_eventManager.unbind_all_inputs();
		}

		void bind_event(auto fn)
		{
			_eventManager.bind_event(fn);
		}

		//void unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event)
		//{
		//	_eventManager.unbind_event(event);
		//}

		void unbind_all_events() { _eventManager.unbind_all_events(); }

		void block_event_type(EEventType type) { _blockedEventTypes.insert(type); }
		void unblock_event_type(EEventType type) { _blockedEventTypes.erase(type); }

		void block_event_category(EEventCategory category) { _blockedEventCategoryMask = _blockedEventCategoryMask | category; }
		void unblock_event_category(EEventCategory category) { _blockedEventCategoryMask = _blockedEventCategoryMask | ~category; }

		void pass_down_event_type(EEventType type) { _notPassedDownEventTypes.erase(type); }
		void unpass_down_event_type(EEventType type) { _notPassedDownEventTypes.insert(type); }

		void pass_down_event_category(EEventCategory category) { _passedDownEventCategoryMask = _blockedEventCategoryMask | category; }
		void unpass_down_event_category(EEventCategory category) { _passedDownEventCategoryMask = _blockedEventCategoryMask | ~category; }

		void pass_down_event(Event& event, u8 include_newly_created_children = false);

		handle<INode> handle_from_this() const { return _handle_from_this; }

		virtual void on_initialized() {};

	private:

		friend struct Application; // TODO: remove friend struct
		void initialize_created_children();
		void cleanup_and_initialize_created_children();
		void initialize_created_children_timestamps(real timestamp);
		void cleanup_destroyed_children();
		void update(real delta);

		u64 _unique_id{ 0 };
		string _name{ "Unnamed" };
		handle<INode> _parent{ nullptr };
		vector<shandle<INode>> _created;
		vector<shandle<INode>> _children;
		r64 _timestamp{ -1.0f };
		
		u8 _isPendingDestruction{false};

		handle<INode> _handle_from_this{ nullptr };
		type_index _type_id{typeid(INode)};

		friend struct EventManager;
		EventManager _eventManager;

		template<typename Include, typename... Stop>
		void dispatch_system_impl(function<void(Include&)> fn, Exclude<Stop...> const& stop, u8 include_newly_created_children) const;

		set<EEventType> _blockedEventTypes{};
		EEventCategory _blockedEventCategoryMask{ 0 };

		set<EEventType> _notPassedDownEventTypes{};
		EEventCategory _passedDownEventCategoryMask{ EEventCategory::Max };
	};

	template<typename T, typename... Args>
	inline handle<T> INode::create(const string& name, const Args&... args)
	{
		__name = name;
		__parent = handle_from_this();
		__type_id = typeid(T);

		void* data = malloc(sizeof(T));
		ASSERT(data != nullptr);

		auto shared = shandle<T>((T*)data); // TODO: may not delete

		__handle_from_this = handle<INode>(shared);

		T* t = new (data) T(args...);

		if (shared)
		{
			_created.push_back(shandle<INode>(shared));
			TRACE("Created child node: ", name);
			__uid++;
		}
		else
		{
			ERROR("Child is nullptr.");
		}

		__parent = nullptr;
		__name = "";
		__handle_from_this = nullptr;
		__type_id = typeid(INode);

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		DebugRenderer::add_object_draw_function<T>();
#endif

		return handle<T>(shared);
	}

	template<typename T>
	inline handle<T> INode::find_parent() const
	{
		handle<INode> parent = _parent;
		while (parent != nullptr)
		{
			if (parent->is_of_type<T>())
			{
				return parent;
			}

			parent = parent->parent();
		}

		return nullptr;
	}

	template<typename Include, typename ...Stop>
	inline void INode::dispatch_system_impl(function<void(Include&)> fn, Exclude<Stop...> const& stop, u8 include_newly_created_children) const
	{
		auto iteration = [&](shandle<INode> const& i)
			{
				ASSERT(i != nullptr);
				if (!i->_isPendingDestruction)
				{
					Include* include = dynamic_cast<Include*>(i.get());

					if (include != nullptr)
					{
						fn(*include);
					}

					u8 block = false;

					([&]
						{
							if (i->is_of_type<Stop>())
							{
								block = true; // TODO: figure out how to exit loop early
							}
						}(), ...);

					if (block)
					{
						return;
					}

					i->dispatch_system_impl(fn, stop, include_newly_created_children);
				}
			};

		for (const auto& i : _children)
		{
			iteration(i);
		}

		if (include_newly_created_children)
		{
			for (const auto& i : _created)
			{
				iteration(i);
			}
		}
	}

	// TODO: make macro for defining print and debug render
	namespace debug
	{
		template<> inline string _Log::_print_format<INode>(INode& v)
		{
			sstream stream;
			stream << "\n";
			stream << "Name: " << v.name() << "\n";
			stream << "Unique id: " << v.unique_id() << "\n";
			//stream << "Parent: name: " << v.parent()->name() << ", unique_id: " << v.parent()->unique_id() << "\n";
			stream << "Num children: " << v.children().size() << "\n";
			stream << "Timestamp: " << v.timestamp() << "\n";
			//stream << "Visibility: " << 
			//	(v.visibility() == ENodeVisibility::Visible ? "Visible"
			//	: v.visibility() == ENodeVisibility::Invisible ? "Invisible"
			//	: "Invisible_Tree")
			//	<< "\n";

			//stream << "Depth: " << v.depth() << "\n";
			stream << "Is pending destruction?: " << (v.is_pending_destruction() ? "True" : "False") << "\n";

			return stream.str();
		}
	}

}