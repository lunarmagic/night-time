
#include "nightpch.h"
#include "Application.h"
#include "core.h"
#include "window/backends/sdl/WindowSDL.h"
//#include "event/MouseEvent.h"
//#include "event/ApplicationEvent.h"
//#include "event/PenEvent.h"
//#include "event/KeyEvent.h"
//#include "input/InputKey.h"
#include "node/INode.h"
#include "node/NodeRenderable.h"
#include "profiler/Profiler.h"
#include "thread/Thread.h"
#include "random/random.h"
#include "node/NodeWindow.h"
#include "action_mapping/ActionMapping.h"

namespace night
{

	//#define BIND_EVENT(x) bind(&Application::x, this, placeholder_1)

	Application* Application::_instance = nullptr;

	Application::Application()
	{
		ASSERT(_instance == nullptr); // Application is a singleton, don't create more than 1
		_instance = this;
	}

	Application::~Application()
	{
	}

	void Application::init()
	{
		init_settings();
		random_set_seed();

		//auto [window, params] = create_window();
		//_window = window;
		_window = create_window();
		_window->init();

		DB_RENDERER_INIT();

		_resourceManager = create_resource_manager();
		ASSERT(_resourceManager != nullptr);
		_resourceManager->load_resources();
		utility::renderer().update_resources();

		create_root();

		on_initialized();

		utility::renderer().update_resources();
		utility::renderer().cleanup_destroyed_resources();
	}

	void Application::init_settings()
	{
		// init settings:
		settings.read("settings/settings.txt");

		File& lil = settings["Night"]["Application"]["Low Input Latency Mode"];
		if (lil.empty())
		{
			lil.set_s32(NIGHT_APPLICATION_DEFAULT_LOW_INPUT_LATENCY_MODE);
			low_input_latency_mode = NIGHT_APPLICATION_DEFAULT_LOW_INPUT_LATENCY_MODE;
		}
		else
		{
			low_input_latency_mode = lil.get_s32();
		}

		File const& am = settings["Night"]["Action Map"];
		for (const auto& i : am.branches())
		{
			string const& action = i.first;
			string const& input_key = i.second.data();

			auto key = string_to_input_key_map.find(input_key);

			if (key != string_to_input_key_map.end())
			{
				action_map.insert({ action, (*key).second });
			}
			else
			{
				WARNING("input key not found in string_to_input_key_map!");
			}
		}
	}

	s32 Application::run()
	{
		init();

		auto dispatch_polled_events = [&]()
			{
				_root->initialize_created_children();
				_root->cleanup_and_initialize_created_children();

				u32 count = 0;
				while (!EventManager::queued_events_empty())
				{
					_eventManager.dispatch_queued_events();

					_root->initialize_created_children();
					_root->cleanup_and_initialize_created_children();

					count++;
					if (count > 128)
					{
						ERROR("exceeded event queue limit");
						break;
					}
				}

				_root->cleanup_destroyed_children();
			};

		while (!_isPendingTermination)
		{
			if (!low_input_latency_mode) // mutlithreaded mode
			{
				NIGHT_PROFILER_PUSH("Application Run-Time");

				sref<RenderGraph> front_graph(new RenderGraph);
				sref<RenderGraph> back_graph(new RenderGraph);

				if (_root)
				{
					NIGHT_PROFILER_PUSH("Window Poll Events");
					_window->poll_events();
					NIGHT_PROFILER_POP();

					Thread<void> test_thread([&]()
						{
							// TODO: there is 1 frame of input delay if we call update before dispatch events.
							NIGHT_PROFILER_PUSH("Application Update");
							_root->update(_window->delta_time());
							NIGHT_PROFILER_POP();

							NIGHT_PROFILER_PUSH("Application Dispatch Events and Cleanup");
							dispatch_polled_events();
							NIGHT_PROFILER_POP();

							// prevent lag spike problems:
							_root->initialize_created_children_timestamps(utility::window().precise_time_elapsed());

							NIGHT_PROFILER_PUSH("Application Render Node Tree");
							NodeRenderable::render_node_tree(_root, *back_graph);
							NIGHT_PROFILER_POP();
						});
					test_thread.wait();

					// threads should join here

					NIGHT_PROFILER_PUSH("Renderer Update Resources");
					utility::renderer().update_resources();
					utility::renderer().cleanup_destroyed_resources();
					NIGHT_PROFILER_POP();

					// TODO: this needs to be fast, because it is done between window poll events
					NIGHT_PROFILER_PUSH("Renderer Flush Node Render Graph");
					SWAP(front_graph, back_graph);
					utility::renderer().flush_render_graph(*front_graph);
					front_graph->clear();
					NIGHT_PROFILER_POP();
				}

				DB_RENDERER_FLUSH();

				NIGHT_PROFILER_PUSH("Window Update");
				_window->update();
				NIGHT_PROFILER_POP();

				NIGHT_PROFILER_POP(); // application run-time

				NIGHT_PROFILER_PUSH("Window Present");
				_window->present();
				NIGHT_PROFILER_POP();
			}
			else // low latency mode
			{
				NIGHT_PROFILER_PUSH("Application Run-Time");

				RenderGraph graph;

				if (_root)
				{
					NIGHT_PROFILER_PUSH("Window Poll Events");
					_window->poll_events();
					NIGHT_PROFILER_POP();

					NIGHT_PROFILER_PUSH("Application Dispatch Events and Cleanup 1");
					dispatch_polled_events();
					NIGHT_PROFILER_POP();

					NIGHT_PROFILER_PUSH("Application Update");
					_root->update(_window->delta_time());
					NIGHT_PROFILER_POP();

					NIGHT_PROFILER_PUSH("Application Dispatch Events and Cleanup 2");
					dispatch_polled_events();
					NIGHT_PROFILER_POP();

					// prevent lag spike problems:
					_root->initialize_created_children_timestamps(utility::window().precise_time_elapsed()); // TODO: this may be bad after update

					NIGHT_PROFILER_PUSH("Application Render Node Tree");
					NodeRenderable::render_node_tree(_root, graph);
					NIGHT_PROFILER_POP();
				}

				NIGHT_PROFILER_PUSH("Renderer Update Resources");
				utility::renderer().update_resources();
				utility::renderer().cleanup_destroyed_resources();
				NIGHT_PROFILER_POP();

				//if (!graph.empty())
				//{
					NIGHT_PROFILER_PUSH("Renderer Flush Node Render Graph");
					utility::renderer().flush_render_graph(graph);
					NIGHT_PROFILER_POP();

					DB_RENDERER_FLUSH();

					NIGHT_PROFILER_PUSH("Window Update");
					_window->update();
					NIGHT_PROFILER_POP();

					
				//}
				//else
				//{
				//	DB_RENDERER_FLUSH();
				//}

				NIGHT_PROFILER_POP(); // application run-time

				NIGHT_PROFILER_PUSH("Window Present");
				_window->present();
				NIGHT_PROFILER_POP();
			}
		}

		close();

		return 0;
	}

	void Application::close()
	{
		on_close();

		_root.reset();

		if (_resourceManager != nullptr)
		{
			_resourceManager->close();
			delete _resourceManager;
		}
		_resourceManager = nullptr;

		if (_window != nullptr)
		{
			_window->close();
			delete _window;
		}
		_window = nullptr;

		settings.write();
	}

	Application& Application::get()
	{
		return *_instance;
	}

	IWindow& Application::window()
	{
		ASSERT(_window != nullptr);
		return *_window;
	}

	IResourceManager& Application::resource_manager()
	{
		ASSERT(_resourceManager != nullptr);
		return *_resourceManager;
	}

	IWindow* Application::create_window()
	{
		// TODO: add default macro
		//return { new WindowSDL, { .title = "Window", /*.width = 640, .height = 480,*/ .eventCallback = [](Event& event) { Application::get().on_event(event); } } };
		return new WindowSDL(WindowParams{.title = "Window", .eventCallback = [](Event& event) { Application::get().on_event(event); } });
	}

	IResourceManager* Application::create_resource_manager()
	{
		return new ResourceManager;
	}

	void Application::create_root()
	{
#if 0
		INode::__name = "Root";
		INode::__parent = nullptr;

		void* data = malloc(sizeof(INode));
		ASSERT(data != nullptr);

		_root = shandle<INode>((INode*)data); // TODO: may not delete

		INode::__handle_from_this = handle<INode>(_root);

		INode* t = new (data) INode();

		INode::__uid++;
		INode::__name = "";
#else
		INode::__name = "Root";
		INode::__parent = nullptr;

		// root should always be a fullscreen NodeWindow.
		void* data = malloc(sizeof(NodeWindow));
		ASSERT(data != nullptr);

		_root = shandle<NodeWindow>((NodeWindow*)data); // TODO: may not delete

		INode::__handle_from_this = handle<INode>(_root);

		NodeWindow* t = new (data) NodeWindow(NodeWindowParams{ .state = ENodeWindowState::Fullscreen });

		INode::__uid++;
		INode::__name = "";
#endif
	}

	//INode* Application::create_root()
	//{
	//	//return new INode();
	//	return new NodeWindow(NodeWindowParams{ .state = ENodeWindowState::Fullscreen });
	//}

	void Application::random_set_seed()
	{
		::night::random_set_seed((u32)time(nullptr));
	}

	void Application::on_event(Event& event)
	{
		_eventManager.on_event(event);

		if (_root)
		{
			_root->on_event(event);
		}
	}

#ifdef false
	void Application::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(on_key_pressed));
		dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(on_key_released));
		dispatcher.dispatch<MouseButtonPressedEvent> (BIND_EVENT(on_mouse_button_pressed));
		dispatcher.dispatch<MouseButtonReleasedEvent> (BIND_EVENT(on_mouse_button_released));
		dispatcher.dispatch<MouseWheelEvent>(BIND_EVENT(on_mouse_wheel));
		dispatcher.dispatch<MouseMotionEvent>(BIND_EVENT(on_mouse_motion));
		dispatcher.dispatch<PenPressureEvent>(BIND_EVENT(on_pen_pressure));
		dispatcher.dispatch<PenDownEvent>(BIND_EVENT(on_pen_down));
		dispatcher.dispatch<PenUpEvent>(BIND_EVENT(on_pen_up));
		dispatcher.dispatch<PenMotionEvent>(BIND_EVENT(on_pen_motion));
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT(on_window_close));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT(on_window_resize));
		dispatcher.dispatch<RendererPresentedEvent>(BIND_EVENT(on_renderer_presented));

		if (_root)
		{
			_root->on_event(event);
		}

		//LOG(event.to_string());
	}

	void Application::callback_bound_inputs(const InputKey& key)
	{
		for (auto [i, end] = _inputBindings.equal_range(key); i != end; i++)
		{
			(*i).second();
		}
	}

	template<typename T>
	inline u8 Application::callback_bound_events(T& event)
	{
		auto r = _eventBindings.equal_range(T::get_static_type());

		for (auto i = r.first; i != r.second; i++)
		{
			(*i).second(event);
		}

		return true;
	}

	u8 Application::on_key_pressed(KeyPressedEvent& event)
	{
		callback_bound_inputs(InputKey( (EKey)event.keycode(), event.isRepeat() ? EInputType::REPEAT : EInputType::PRESSED));
		return callback_bound_events(event);
	}

	u8 Application::on_key_released(KeyReleasedEvent& event)
	{
		callback_bound_inputs(InputKey((EKey)event.keycode(), EInputType::RELEASED ));
		return callback_bound_events(event);
	}

	u8 Application::on_mouse_button_pressed(MouseButtonPressedEvent& event)
	{
		callback_bound_inputs(InputKey( (EMouse)event.button(), EInputType::PRESSED ));
		return callback_bound_events(event);
	}

	u8 Application::on_mouse_button_released(MouseButtonReleasedEvent& event)
	{
		callback_bound_inputs(InputKey( (EMouse)event.button(), EInputType::RELEASED ));
		return callback_bound_events(event);
	}

	u8 Application::on_mouse_wheel(MouseWheelEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_pen_pressure(PenPressureEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_pen_down(PenDownEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_pen_up(PenUpEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_pen_motion(PenMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_mouse_motion(MouseMotionEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_window_close(WindowCloseEvent& event)
	{
		u8 result = callback_bound_events(event);

		terminate();

		return result;
	}

	u8 Application::on_window_resize(WindowResizeEvent& event)
	{
		return callback_bound_events(event);
	}

	u8 Application::on_renderer_presented(RendererPresentedEvent& event)
	{
		return callback_bound_events(event);
	}

	umultimap<InputKey, function<void()>>::iterator Application::bind_input(EKey key, EInputType type, function<void()> fn)
	{
		InputKey input_key(key, type);
		return _inputBindings.insert({ input_key, fn });
	}

	umultimap<InputKey, function<void()>>::iterator Application::bind_input(EMouse mouse, EInputType type, function<void()> fn)
	{
		InputKey input_key(mouse, type);
		return _inputBindings.insert({ input_key, fn });
	}

	umultimap<InputKey, function<void()>>::iterator Application::bind_input(EButton button, EInputType type, function<void()> fn)
	{
		InputKey input_key(button, type);
		return _inputBindings.insert({ input_key, fn });
	}

	void Application::unbind_input(umultimap<InputKey, function<void()>>::iterator input)
	{
		_inputBindings.erase(input);
	}

	void Application::unbind_all_inputs()
	{
		_inputBindings.clear();
	}

	void Application::unbind_event(umultimap<EEventType, function<void(Event&)>>::iterator event)
	{
		_eventBindings.erase(event);
	}

	void Application::unbind_all_events()
	{
		_eventBindings.clear();
	}
#endif

}