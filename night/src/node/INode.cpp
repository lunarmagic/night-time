
#include "nightpch.h"
#include "INode.h"
#include "log/log.h"
#include "window/IWindow.h"
#include "utility.h"
#include "event/MouseEvent.h"
#include "event/PenEvent.h"
#include "event/ApplicationEvent.h"
#include "event/KeyEvent.h"
#include "event/NodeEvent.h"
#include "NodeRenderable.h"
#include "texture/ITexture.h"
#include "node/NodeRenderTarget.h"

//#define BIND_EVENT(x) bind(&INode::x, this, placeholder_1)

namespace night
{

	u64 INode::__uid = 0;
	string INode::__name = "";
	handle<INode> INode::__parent = nullptr;
	handle<INode> INode::__handle_from_this = nullptr;
	type_index INode::__type_id = typeid(INode);

	INode::INode()
		: _type_id(__type_id)
	{
		_unique_id = __uid;
		_name = __name;
		_parent = __parent;
		_handle_from_this = __handle_from_this;
		is_active = true;
		is_taking_events = true;
		_timestamp = -1.0f;
		lifespan = -1.0f;
		_isPendingDestruction = false;
	}

	INode::~INode()
	{
		TRACE("Destroying ", _name);
		_children.clear();
		_created.clear();

		//remove_all();
	}

	void INode::reset_timestamp()
	{
		//utility::window().update_time_elapsed(); // TODO
		//_timestamp = utility::window().time_elapsed();
		_timestamp = utility::window().precise_time_elapsed();
	}

	r64 INode::time_elapsed() const
	{
		if (_timestamp == -1)
		{
			return 0;
		}

		return utility::window().time_elapsed() - _timestamp;
	}

	void INode::initialize_created_children()
	{
		// properly add newly created children
		for (const auto& i : _created)
		{
			ASSERT(i != nullptr);
			//i->_timestamp = utility::window().time_elapsed(); // TODO: move out of here
			_children.push_back(i);
		}

		//_created.clear();

		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			i->initialize_created_children();
		}
	}

	void INode::cleanup_and_initialize_created_children() // TODO: combine with initialize_created_children
	{
		for (const auto& i : _created)
		{
			ASSERT(i != nullptr);
			i->on_initialized();
		}

		_created.clear();

		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			i->cleanup_and_initialize_created_children();
		}
	}

	// TODO: use vector for initializing and timestamping created.
	void INode::initialize_created_children_timestamps(real timestamp)
	{
		if (_timestamp == -1)
		{
			_timestamp = timestamp; // TODO: move out of here
		}
		
		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			i->initialize_created_children_timestamps(timestamp);
		}
	}

	void INode::cleanup_destroyed_children()
	{
		for (auto i = _children.begin(); i != _children.end();)
		{
			ASSERT((*i) != nullptr);
			if ((*i)->_isPendingDestruction)
			{
				i = _children.erase(i);
			}
			else
			{
				(*i)->cleanup_destroyed_children();

				if ((*i)->_isPendingDestruction) // lifespan may have run out, or it was destroyed in a child nodes destructor
				{
					i = _children.erase(i);
				}
				else
				{
					i++;
				}
			}
		}

		// if lifespan runs out, destroy
		if (lifespan != -1.0f)
		{
			r64 time_elapsed = utility::window().time_elapsed() - _timestamp;

			if (time_elapsed > lifespan)
			{
				destroy();
			}
		}
	}

	void INode::update(real delta)
	{
		// update
		if (!is_active)
		{
			return;
		}

		on_update(delta);

		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			i->update(delta);
		}
	}

	void INode::remove(const string& name)
	{
#ifdef false
		auto fn = [&](auto& v)
		{
				for (const auto& i : v)
				{
					ASSERT(i != nullptr);
					if (i->name() == name)
					{
						i->destroy();
					}
				}
#ifdef false
			for (auto i = v.begin(); i != v.end(); )
			{
				if ((*i) != nullptr)
				{
					if ((*i)->name() == name) // TODO: fix crash
					{
						i = v.erase(i);
					}
					else
					{
						i++;
					}
				}
				else
				{
					i = v.erase(i);
				}
			}
#endif
		};

		fn(_children);
		fn(_created);
#endif

		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			if (i->name() == name)
			{
				i->destroy();
			}
		}

		for (const auto& i : _created)
		{
			ASSERT(i != nullptr);
			if (i->name() == name)
			{
				i->destroy();
			}
		}
	}

	void INode::remove_all()
	{
		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			i->destroy();
		}

		for (const auto& i : _created)
		{
			ASSERT(i != nullptr);
			i->destroy();
		}
	}

	handle<INode> INode::find(const string& name) const
	{
		// TODO: update this function.
		//for (auto i = _children.begin(); i < _children.end(); i++)
		//{
		//	if ((*i) != nullptr)
		//	{
		//		if ((*i)->name() == name)
		//		{
		//			return (*i);
		//		}
		//	}
		//	else
		//	{
		//		// remove child
		//	}
		//}

		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			if (i->name() == name)
			{
				return i;
			}
		}

		return nullptr;
	}

	void INode::destroy()
	{
		_isPendingDestruction = true;
	}

	void INode::move(handle<INode> new_parent)
	{
		// TODO: look at this function.
		if (parent() == new_parent)
		{
			WARNING("moved node to it's own parent.");
			return;
		}

		ASSERT(parent() != nullptr);
		handle<INode> const& parent = this->parent();

		shandle<INode> ref = handle_from_this();
		ASSERT(ref != nullptr);

		u8 is_child = false;

		for (auto i = parent->_children.begin(); i != parent->_children.end();)
		{
			if ((*i).get() == this)
			{
				i = parent->_children.erase(i);
				is_child = true;
			}
			else
			{
				i++;
			}
		}

		if (!is_child)
		{
			for (auto i = parent->_created.begin(); i != parent->_created.end();)
			{
				if ((*i).get() == this)
				{
					i = parent->_created.erase(i);
				}
				else
				{
					i++;
				}
			}

			new_parent->_created.push_back(ref);
			TRACE("moved created node ", name(), "from ", parent->name(), ", to ", new_parent->name());
		}
		else
		{
			new_parent->_children.push_back(ref);
			TRACE("moved child node ", name(), "from ", parent->name(), ", to ", new_parent->name());
		}
		
		NodeMovedEvent event(parent, new_parent);
		on_event(event);
	}


	// EVENT SYSTEM:
	void INode::on_event(Event& event)
	{
		if (!is_taking_events || !is_active)
		{
			return;
		}

		// TODO: handle blocked categories.
		if (_blockedEventTypes.find(event.type()) == _blockedEventTypes.end())
		{
			_eventManager.on_event(handle_from_this(), event);
		}

		// TODO: node can't differentiate between parents NodeMovedEvents or it's own

		// TODO: handle blocked categories.

		if (_notPassedDownEventTypes.find(event.type()) != _notPassedDownEventTypes.end())
		{
			return;
		}

		pass_down_event(event);

	}

	void INode::pass_down_event(Event& event, u8 include_newly_created_children)
	{
#ifdef false
		for (auto i = _children.begin(); i != _children.end();)
		{
			auto& child = (*i);
			if (child != nullptr && !child->_isPendingDestruction)
			{
				child->on_event(event);
				i++;
			}
			else
			{
				i = _children.erase(i);
			}
		}
#endif
		for (const auto& i : _children)
		{
			ASSERT(i != nullptr);
			i->on_event(event);
		}

		if (include_newly_created_children)
		{
			for (const auto& i : _created) 
			{
				ASSERT(i != nullptr);
				i->on_event(event);
			}
		}
	}

	s32 INode::depth_from_root() const
	{
		s32 depth = 0;

		handle<INode> parent = this->parent();
		while (parent != nullptr)
		{
			depth++;
			parent = parent->parent();
		}

		return depth;
	}

#ifdef false

	void INode::callback_bound_inputs(const InputKey& key)
	{
		for (auto [i, end] = _inputBindings.equal_range(key); i != end; i++)
		{
			(*i).second();
		}
	}

	template<typename T>
	inline u8 INode::callback_bound_events(T& event)
	{
		auto r = _eventBindings.equal_range(T::get_static_type());

		for (auto i = r.first; i != r.second; i++)
		{
			(*i).second(event);
		}

		return true;
	}

	u8 INode::on_key_pressed(KeyPressedEvent& event)
	{
		callback_bound_inputs(InputKey((EKey)event.keycode(), event.isRepeat() ? EInputType::REPEAT : EInputType::PRESSED));
		return callback_bound_events(event);
	}

	u8 INode::on_key_released(KeyReleasedEvent& event)
	{
		callback_bound_inputs(InputKey((EKey)event.keycode(), EInputType::RELEASED));
		return callback_bound_events(event);
	}

	u8 INode::on_mouse_button_pressed(MouseButtonPressedEvent& event)
	{
		callback_bound_inputs(InputKey((EMouse)event.button(), EInputType::PRESSED));
		return callback_bound_events(event);
	}

	u8 INode::on_mouse_button_released(MouseButtonReleasedEvent& event)
	{
		callback_bound_inputs(InputKey((EMouse)event.button(), EInputType::RELEASED));
		return callback_bound_events(event);
	}

	u8 INode::on_mouse_wheel(MouseWheelEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_pen_pressure(PenPressureEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_pen_down(PenDownEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_pen_up(PenUpEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_pen_motion(PenMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_mouse_motion(MouseMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_window_close(WindowCloseEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_window_resize(WindowResizeEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_renderer_presented(RendererPresentedEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 INode::on_node_moved(NodeMovedEvent& event)
	{
		return callback_bound_events(event);
	}

	umultimap<InputKey, function<void()>>::iterator INode::bind_input(EKey key, EInputType type, function<void()> fn)
	{
		InputKey input_key(key, type);
		return _inputBindings.insert({ input_key, fn });
	}

	umultimap<InputKey, function<void()>>::iterator INode::bind_input(EMouse mouse, EInputType type, function<void()> fn)
	{
		InputKey input_key(mouse, type);
		return _inputBindings.insert({ input_key, fn });
	}

	umultimap<InputKey, function<void()>>::iterator INode::bind_input(EButton button, EInputType type, function<void()> fn)
	{
		InputKey input_key(button, type);
		return _inputBindings.insert({ input_key, fn });
	}

	void INode::unbind_input(umultimap<InputKey, function<void()>>::iterator input)
	{
		_inputBindings.erase(input);
	}

	void INode::unbind_all_inputs()
	{
		_inputBindings.clear();
	}

	void INode::unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event)
	{
		_eventBindings.erase(event);
	}

	void INode::unbind_all_events()
	{
		_eventBindings.clear();
	}
#endif

	handle<const ITexture> INode::current_render_target() const
	{
		handle<INode> parent = this->parent();
		if (parent == nullptr)
		{
			return utility::renderer().default_render_target();
		}

		return parent->current_render_target_impl();

		//handle<INode> parent = this->parent();
		//while (parent != nullptr)
		//{
		//	NodeRenderTarget* dc = dynamic_cast<NodeRenderTarget*>(parent.ptr().lock().get());
		//	if (dc != nullptr)
		//	{
		//		return dc->target();
		//	}

		//	parent = parent->parent();
		//}

		//return utility::renderer().default_render_target();
	}

	handle<const ITexture> INode::current_render_target_impl() const
	{
		if (is_of_type<NodeRenderTarget>())
		{
			return ((NodeRenderTarget*)this)->target();
		}

		auto parent = this->parent();
		if (parent == nullptr)
		{
			return utility::renderer().default_render_target();
		}

		return parent->current_render_target_impl();
	}

}