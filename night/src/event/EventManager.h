#pragma once

#include "event/Event.h"
#include "input/InputKey.h"
#include "handle/handle.h"

namespace night
{
	//struct Event;
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
	struct INode;
	//struct InputKey;

	struct EventManager
	{
		//void begin_taking_events();
		void on_event(Event& event);
		// call within node event handling to make sure the handle is valid before calling the callback
		void on_event(handle<INode> node_handle, Event& event);
		//void end_taking_events();

		static u8 queued_events_empty() { return EventManager::_eventsToBeDispatched.empty(); }
		static void dispatch_queued_events();

		void bind_input(EKey key, EInputType type, function<void()> fn);
		void bind_input(EMouse mouse, EInputType type, function<void()> fn);
		void bind_input(EButton button, EInputType type, function<void()> fn);

		void bind_input(string const& action, EInputType type, function<void()> fn);

		// pass in the iterator returned from bind_input.
		//void unbind_input(umultimap<InputKey, function<void()>>::iterator input);
		void unbind_all_inputs();

		umultimap<EEventType, function<void(Event&)>>::iterator bind_event(auto fn)
		{
			return bind_event_impl(function(fn));
		}

		//void unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event);
		void unbind_all_events();

		//void block_event_type(EEventType type) { _blockedEventTypes.insert(type); }
		//void unblock_event_type(EEventType type) { _blockedEventTypes.erase(type); }

		//void block_event_category(EEventCategory category) { _blockedEventCategoryMask = _blockedEventCategoryMask | category; }
		//void unblock_event_category(EEventCategory category) { _blockedEventCategoryMask = _blockedEventCategoryMask | ~category; }

	private:

		template<typename T>
		umultimap<EEventType, function<void(Event&)>>::iterator bind_event_impl(function<void(T&)> fn)
		{
			auto l = [fn](Event& event)
				{
					if (event.type() == T::get_static_type())
					{
						fn(static_cast<T&>(event));
					}
				};

			return _eventBindings.insert({ T::get_static_type(), l });
		}

		u8 on_event(KeyPressedEvent& event);
		u8 on_event(KeyReleasedEvent& event);
		u8 on_event(MouseButtonPressedEvent& event);
		u8 on_event(MouseButtonReleasedEvent& event);
		u8 on_event(MouseMotionEvent& event);
		u8 on_event(MouseWheelEvent& event);
		u8 on_event(PenPressureEvent& event);
		u8 on_event(PenDownEvent& event);
		u8 on_event(PenUpEvent& event);
		u8 on_event(PenMotionEvent& event);
		u8 on_event(WindowCloseEvent& event);
		u8 on_event(WindowResizeEvent& event);
		u8 on_event(RendererPresentedEvent& event);

		// TODO: handle ports for controllers, handle axis mapping
		void callback_bound_inputs(const InputKey& key);
		template<typename T> u8 callback_bound_events(T& event);

		umultimap<InputKey, function<void()>> _inputBindings; // TODO: may want to combine input bindings and event bindings.
		umultimap<EEventType, function<void(Event&)>> _eventBindings;
		
		static vector<function<void()>> _eventsToBeDispatched;
		//u8 _isTakingEvents{ false };
	};
}