#pragma once

//#include "utility.h"
#include "core.h"
#include "input/InputKey.h"
#include "event/Event.h"
//#include "ref/ref.h"
#include "handle/handle.h"
#include "event/EventManager.h"
#include "file/File.h"

#define NIGHT_APPLICATION_DEFAULT_LOW_INPUT_LATENCY_MODE false

namespace night
{

	struct IWindow;
	struct IResourceManager;
	struct Event;
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
	struct InputKey;

	struct WindowParams;

	struct NIGHT_API Application
	{
		Application();
		~Application();

		s32 run();

		static Application& get();

		IWindow& window();
		IResourceManager& resource_manager();
		//Gui& gui() { return *_gui; }

		void terminate() { _isPendingTermination = true; }

		void pause() { _isPaused = true; _isPendingFrameAdvance = false; }
		void unpause() { _isPaused = false; _isPendingFrameAdvance = false; }
		b8 const& is_paused() const { return _isPaused; }
		void frame_advance() { if(_isPaused) _isPendingFrameAdvance = true; };

		void fixed_update_step_size(real size) { _fixedUpdateStepSize = size; };
		real const& fixed_update_step_size() const { return _fixedUpdateStepSize; }

		handle<INode> const root() { return _root; }

		// TODO: use action mapping
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

		void unbind_all_events()
		{
			_eventManager.unbind_all_events();
		}

#ifdef false

		// TODO: fix this
		// returns an iterator that is used to unbind the input later.
		umultimap<InputKey, function<void()>>::iterator bind_input(EKey key, EInputType type, function<void()> fn);
		umultimap<InputKey, function<void()>>::iterator bind_input(EMouse mouse, EInputType type, function<void()> fn);
		umultimap<InputKey, function<void()>>::iterator bind_input(EButton button, EInputType type, function<void()> fn);

		// pass in the iterator returned from bind_input.
		void unbind_input(umultimap<InputKey, function<void()>>::iterator input);
		void unbind_all_inputs();

		umultimap<EEventType, function<void(Event&)>>::iterator bind_event(auto fn)
		{
			return bind_event_impl(function(fn));
		}

		void unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event);
		void unbind_all_events();
#endif

		void queue_for_main_thread(function<void()> callback);

		// TODO: make singletons
		File settings;
		umultimap<string, InputKey> action_map;
		b8 low_input_latency_mode{ NIGHT_APPLICATION_DEFAULT_LOW_INPUT_LATENCY_MODE };

	protected:

		// TODO: may want to use sref
		//virtual pair<IWindow*, WindowParams> create_window();
		virtual IWindow* create_window();
		virtual IResourceManager* create_resource_manager();
		
		virtual void random_set_seed();

		//virtual void on_load_resources() { return; }
		virtual void on_initialized() { return; }
		virtual void on_close() { return; }

		void dispatch_main_thread_callbacks();
	
	private:

		//void load_resources();
		void init();
		void init_settings();
		void close();

		// TODO: make event handler struct
		void on_event(Event& event);

#ifdef false
		b8 on_key_pressed(KeyPressedEvent& event);
		b8 on_key_released(KeyReleasedEvent& event);
		b8 on_mouse_button_pressed(MouseButtonPressedEvent& event);
		b8 on_mouse_button_released(MouseButtonReleasedEvent& event);
		b8 on_mouse_motion(MouseMotionEvent& event);
		b8 on_mouse_wheel(MouseWheelEvent& event);
		b8 on_pen_pressure(PenPressureEvent& event);
		b8 on_pen_down(PenDownEvent& event);
		b8 on_pen_up(PenUpEvent& event);
		b8 on_pen_motion(PenMotionEvent& event);
		b8 on_window_close(WindowCloseEvent& event);
		b8 on_window_resize(WindowResizeEvent& event);
		b8 on_renderer_presented(RendererPresentedEvent& event);

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

		void callback_bound_inputs(const InputKey& key);
		template<typename T> b8 callback_bound_events(T& event);

		umultimap<InputKey, function<void()>> _inputBindings; // TODO: may want to combine input bindings and event bindings.
		umultimap<EEventType, function<void(Event&)>> _eventBindings;
#endif

		IWindow* _window{ nullptr };
		IResourceManager* _resourceManager{ nullptr };
		friend struct EventManager;
		EventManager _eventManager;
		b8 _isPendingTermination{ false };

		b8 _isPaused = false;
		b8 _isPendingFrameAdvance = false;

		real _fixedUpdateAccumulatedDeltaTime = 0.0f;
		real _fixedUpdateStepSize = 1.0f / 60.0f;

		deque<function<void()>> _mainThreadCallbacks = {};

		shandle<INode> _root;
		void create_root();

		static Application* _instance;

		friend struct WindowParams;
		friend struct DebugRenderer;
	};

	struct WindowParams
	{
		string title{ "Window" };
		s32 width{ -1 };
		s32 height{ -1 };
		real fps{ -1 };
		function<void(Event&)> eventCallback{ [](Event& event) { Application::get().on_event(event); } };
	};

	Application* create_application();

}