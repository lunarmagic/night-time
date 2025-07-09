
#include "nightpch.h"
#include "EventManager.h"
#include "event/MouseEvent.h"
#include "event/ApplicationEvent.h"
#include "event/PenEvent.h"
#include "event/KeyEvent.h"
#include "input/InputKey.h"
#include "application/Application.h"
#include "node/INode.h"

#define BIND_EVENT(x) bind(&EventManager::x, this, placeholder_1)

namespace night
{
	vector<function<void()>> EventManager::_eventsToBeDispatched;

	void EventManager::on_event(Event& event)
	{
		auto fn = [&]<typename E>(E & e)
		{
			if (e.type() == E::get_static_type())
			{
				E e_copy = e;

				auto fn2 = [this, e_copy]() mutable
				{
					Application::get()._eventManager.on_event(e_copy);
				};

				EventManager::_eventsToBeDispatched.push_back(function(fn2));
			}
		};

		fn((KeyPressedEvent&)event);
		fn((KeyReleasedEvent&)event);
		fn((MouseButtonPressedEvent&)event);
		fn((MouseButtonReleasedEvent&)event);
		fn((MouseWheelEvent&)event);
		fn((MouseMotionEvent&)event);
		fn((PenPressureEvent&)event);
		fn((PenDownEvent&)event);
		fn((PenUpEvent&)event);
		fn((PenMotionEvent&)event);
		fn((WindowCloseEvent&)event);
		fn((WindowResizeEvent&)event);
		fn((RendererPresentedEvent&)event);

#ifdef false
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(on_key_pressed));
		dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(on_key_released));
		dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT(on_mouse_button_pressed));
		dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT(on_mouse_button_released));
		dispatcher.dispatch<MouseWheelEvent>(BIND_EVENT(on_mouse_wheel));
		dispatcher.dispatch<MouseMotionEvent>(BIND_EVENT(on_mouse_motion));
		dispatcher.dispatch<PenPressureEvent>(BIND_EVENT(on_pen_pressure));
		dispatcher.dispatch<PenDownEvent>(BIND_EVENT(on_pen_down));
		dispatcher.dispatch<PenUpEvent>(BIND_EVENT(on_pen_up));
		dispatcher.dispatch<PenMotionEvent>(BIND_EVENT(on_pen_motion));
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT(on_window_close));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT(on_window_resize));
		dispatcher.dispatch<RendererPresentedEvent>(BIND_EVENT(on_renderer_presented));
#endif
	}

	void EventManager::on_event(handle<INode> node_handle, Event& event)
	{
		auto fn = [&]<typename E>(E & e)
		{
			if (e.type() == E::get_static_type())
			{
				E e_copy = e;

				auto fn2 = [node_handle, e_copy]() mutable
				{
					if (node_handle != nullptr)
					{
						node_handle->_eventManager.on_event(e_copy);
					}
				};

				EventManager::_eventsToBeDispatched.push_back(function(fn2));
			}
		};

		fn((KeyPressedEvent&)event);
		fn((KeyReleasedEvent&)event);
		fn((MouseButtonPressedEvent&)event);
		fn((MouseButtonReleasedEvent&)event);
		fn((MouseWheelEvent&)event);
		fn((MouseMotionEvent&)event);
		fn((PenPressureEvent&)event);
		fn((PenDownEvent&)event);
		fn((PenUpEvent&)event);
		fn((PenMotionEvent&)event);
		fn((WindowCloseEvent&)event);
		fn((WindowResizeEvent&)event);
		fn((RendererPresentedEvent&)event);
	}

	void EventManager::dispatch_queued_events()
	{
		vector<function<void()>> temp;

		EventManager::_eventsToBeDispatched.swap(temp);

		for (const auto& i : temp)
		{
			ASSERT(i != nullptr);
			i();
		}

		temp.clear();
	}

	void EventManager::callback_bound_inputs(const InputKey& key)
	{
		for (auto [i, end] = _inputBindings.equal_range(key); i != end; i++)
		{
			(*i).second();
		}
	}

	template<typename T>
	inline u8 EventManager::callback_bound_events(T& event)
	{
		auto r = _eventBindings.equal_range(T::get_static_type());

		for (auto i = r.first; i != r.second; i++)
		{
			(*i).second(event);
		}

		return true;
	}

	u8 EventManager::on_event(KeyPressedEvent& event)
	{
		callback_bound_inputs(InputKey((EKey)event.keycode(), event.isRepeat() ? EInputType::Repeat : EInputType::Pressed));
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(KeyReleasedEvent& event)
	{
		callback_bound_inputs(InputKey((EKey)event.keycode(), EInputType::Released));
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(MouseButtonPressedEvent& event)
	{
		callback_bound_inputs(InputKey((EMouse)event.button(), EInputType::Pressed));
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(MouseButtonReleasedEvent& event)
	{
		callback_bound_inputs(InputKey((EMouse)event.button(), EInputType::Released));
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(MouseWheelEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(PenPressureEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(PenDownEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(PenUpEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(PenMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(MouseMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(WindowCloseEvent& event)
	{
		u8 result = callback_bound_events(event);

		Application::get().terminate();

		return result;
	}

	u8 EventManager::on_event(WindowResizeEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 EventManager::on_event(RendererPresentedEvent& event)
	{
		return callback_bound_events(event);
	}

	void EventManager::bind_input(EKey key, EInputType type, function<void()> fn)
	{
		InputKey input_key(key, type);
		_inputBindings.insert({ input_key, fn });
	}

	void EventManager::bind_input(EMouse mouse, EInputType type, function<void()> fn)
	{
		InputKey input_key(mouse, type);
		_inputBindings.insert({ input_key, fn });
	}

	void EventManager::bind_input(EButton button, EInputType type, function<void()> fn)
	{
		InputKey input_key(button, type);
		_inputBindings.insert({ input_key, fn });
	}

	void EventManager::bind_input(string const& action, EInputType type, function<void()> fn)
	{
		auto& action_map = utility::action_map();

		for (auto [i, end] = action_map.equal_range(action); i != end; i++)
		{
			_inputBindings.insert({ (*i).second, fn });
		}
	}

	//void EventManager::unbind_input(umultimap<InputKey, function<void()>>::iterator input)
	//{
	//	_inputBindings.erase(input);
	//}

	void EventManager::unbind_all_inputs()
	{
		_inputBindings.clear();
	}

	//void EventManager::unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event)
	//{
	//	_eventBindings.erase(event);
	//}

	void EventManager::unbind_all_events()
	{
		_eventBindings.clear();
	}

}