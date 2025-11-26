
#include "nightpch.h"

//#include <SDL3/SDL_main.h>
//#undef _main

#include <SDL3/SDL.h>

#include <SDL3_image/SDL_image.h> // TODO: figure out why SDL_image is broken.

#include "WindowSDL.h"
#include "application/Application.h"
#include "log/log.h"
#include "geometry/Quad.h"
#include "color/Color.h"
#include "event/MouseEvent.h"
#include "event/ApplicationEvent.h"
#include "event/KeyEvent.h"
#include "event/PenEvent.h"
#include "renderer/IRenderer.h"
#include "renderer/backends/sdl/RendererSDL.h"
#include "renderer/backends/opengl/RendererOpenGL.h"
#include "gui/Gui.h"
#include "sdl_scancode_map.h"
#include "profiler/Profiler.h"
#include "debug_renderer/DebugRenderer.h"

namespace night
{

	s32 WindowSDL::init()
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			ERROR("SDL failed to initialize! SDL_Error: {0}", SDL_GetError());
			return -1;
		}

		//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		// TODO: get renderer type
		_sdlWindow = SDL_CreateWindow(title().c_str(), width(), height(), /*SDL_WINDOW_SHOWN |*/ SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL); // TODO: add flags to params
		if (_sdlWindow == NULL)
		{
			ERROR("SDL Window failed to initialize! SDL_Error: {0}", SDL_GetError());
			return -1;
		}

		if (__super::init() < 1)
		{
			return -1;
		}

		TRACE("Window Initialized.");

		_startTick = (u64)SDL_GetTicks();
		_frameTick = _startTick;
		
		_gui = new Gui; // TODO: make create_gui function
		if (_gui->init() < 0)
		{
			delete _gui;
			_gui = new IGui; // create a blank gui so our gui code does not break.
		}

		float x, y;
		_mouseButtonState = (u32)SDL_GetMouseState(&x, &y);
		_mousePosition.x = (real)x;
		_mousePosition.y = (real)y;

		_mousePosition.y = height() - _mousePosition.y; // flip coord for opengl
		_isMouseGrabbed = SDL_GetWindowMouseGrab(_sdlWindow);
		_isCursorVisible = SDL_CursorVisible();
		_isFullscreen = false;

		return 1;
	}

	void WindowSDL::close()
	{
		_gui->close();

		__super::close();
		SDL_DestroyWindow(_sdlWindow);
		SDL_Quit();
	}

	void WindowSDL::poll_events()
	{
		_mouseMotion = vec2(0);
		auto& callback = event_callback();

		if (callback)
		{
			SDL_Event event;

			while (SDL_PollEvent(&event))
			{
				GuiPollEventResult gui_result;

				if (_gui)
				{
					gui_result = _gui->poll_event(event);
				}

				switch (event.type)
				{
				case SDL_EVENT_QUIT:
				{
					WindowCloseEvent e;
					callback(e);
					break;
				}

				case SDL_EVENT_MOUSE_MOTION:
				{
					//if (!gui_result.wants_mouse_capture) // may not want this
					{
						//MouseMotionEvent e((real)event.motion.xrel, (real)event.motion.yrel);
						ivec2 internal = vec2(event.motion.xrel, event.motion.yrel);
						//vec2 local = internal_to_local(internal);

						s32 w = width();
						s32 h = height();

						vec2 local = { (real)internal.x, (real)internal.y };
						local.x /= (real)w / 2;
						local.y /= (real)h / 2;
						local.y = -local.y;

						_mouseMotion += local;

						MouseMotionEvent e(local);
						callback(e);
					}
					break;
				}

				case SDL_EVENT_WINDOW_RESIZED:
				{
					WindowResizeEvent e(event.window.data1, event.window.data2);
					renderer().on_window_resize(event.window.data1, event.window.data2);
					on_resize(e); // TODO: may want to call this from application.
					callback(e);
					break;
				}
				}

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
				if (DebugRenderer::wants_input())
				{
					continue;
				}
#endif

				switch (event.type)
				{
				case SDL_EVENT_KEY_DOWN: // TODO: map sdl keycodes to our own keycodes
				{
					if (!gui_result.wants_keyboard_capture)
					{
						KeyPressedEvent e(sdl_scancode_map[event.key.scancode], event.key.repeat != 0);
						callback(e);
					}

					break;
				}

				case SDL_EVENT_KEY_UP:
				{
					if (!gui_result.wants_keyboard_capture) // may not want this
					{
						KeyReleasedEvent e(sdl_scancode_map[event.key.scancode]);
						callback(e);
					}

					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					if (!gui_result.wants_mouse_capture)
					{
						ivec2 internal = vec2(event.ptouch.x, event.ptouch.y);
						vec2 local = internal_to_local(internal);
						local.y = -local.y; // flip for opengl
						MouseButtonPressedEvent e(sdl_mouse_map[event.button.button], local);
						callback(e);
					}

					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					if (!gui_result.wants_mouse_capture) // may not want this
					{
						ivec2 internal = vec2(event.ptouch.x, event.ptouch.y);
						vec2 local = internal_to_local(internal);
						local.y = -local.y; // flip for opengl
						MouseButtonReleasedEvent e(sdl_mouse_map[event.button.button], local);
						callback(e);
					}

					break;
				}

				case SDL_EVENT_MOUSE_WHEEL:
				{
					MouseWheelEvent e((real)event.wheel.x, (real)event.wheel.y);
					callback(e);
					break;
				}

				case SDL_EVENT_PEN_AXIS:
				{
					PenPressureEvent e((real)event.paxis.value, (s32)event.paxis.which);
					callback(e);
					break;
				}

				case SDL_EVENT_PEN_DOWN:
				{
					ivec2 internal = vec2(event.ptouch.x, event.ptouch.y);
					vec2 local = internal_to_local(internal);
					local.y = -local.y; // flip for opengl
					PenDownEvent e(local, event.ptouch.eraser, event.ptouch.which);
					callback(e);
					break;
				}

				case SDL_EVENT_PEN_UP:
				{
					ivec2 internal = vec2(event.ptouch.x, event.ptouch.y);
					vec2 local = internal_to_local(internal);
					local.y = -local.y; // flip for opengl
					PenUpEvent e(local, event.ptouch.eraser, event.ptouch.which);
					callback(e);
					break;
				}

				case SDL_EVENT_PEN_MOTION:
				{
					ivec2 internal = vec2(event.pmotion.x, event.pmotion.y);
					vec2 local = internal_to_local(internal);
					local.y = -local.y; // flip for opengl
					PenMotionEvent e(local, event.ptouch.which);
					callback(e);
					break;
				}
				}
			}
		}
	}

	void WindowSDL::update() // TODO: map sdl events to our own events
	{
		float x, y;
		_mouseButtonState = (u32)SDL_GetMouseState(&x, &y);
		_mousePosition.x = (real)x;
		_mousePosition.y = (real)y;

		_mousePosition.y = height() - _mousePosition.y; // flip coord for opengl

		renderer().update();
	}

	void WindowSDL::present()
	{
		if (_gui)
		{
			_gui->update();
		}

		renderer().present();

		u64 delta_ticks = (u64)SDL_GetTicks() - _frameTick;

		// end fps timer
		static std::chrono::time_point<std::chrono::high_resolution_clock> fps_start_time = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> fps_current_time = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(fps_start_time).time_since_epoch().count();
		auto current = std::chrono::time_point_cast<std::chrono::microseconds>(fps_current_time).time_since_epoch().count();
		u64 duration = (u64)(current - start);
		r64 fps_delta_time = 1.0 / ((r64)duration / 1000000.0);
		if (std::_Is_nan(math::lerp(_framerate, (real)fps_delta_time, 0.01f)))
		{
			_framerate = (real)fps_delta_time;
		}
		else
		{
			_framerate = math::lerp(_framerate, (real)fps_delta_time, 0.01f);
		}
		

#ifndef NIGHT_DIST
		string str = "fps: " + std::to_string(_framerate).substr(0, 5);
		SDL_SetWindowTitle(_sdlWindow, str.c_str());
#endif

		if (u32(1000.0f / fps()) > delta_ticks)
		{
			SDL_Delay(u32(1000.0 / fps()) - delta_ticks);
		}

		// start fps timer
		fps_start_time = std::chrono::high_resolution_clock::now();
				
		delta_time((real)(SDL_GetTicks() - _frameTick) / 1000.0f);

		_frameTick = (u64)SDL_GetTicks();
		time_elapsed(time_elapsed() + (r64)delta_time());
	}

	vec2 WindowSDL::mouse() const
	{
		//r32 x;
		//r32 y;
		//SDL_GetMouseState(&x, &y);
		//
		//y = height() - y; // flip coord for opengl

		return internal_to_local({ _mousePosition.x, _mousePosition.y });
	}

	b8 WindowSDL::mouse_down(EMouse mouse) const
	{
		//r32 x;
		//r32 y;
		//SDL_MouseButtonFlags flags = SDL_GetMouseState(&x, &y);

#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		if (DebugRenderer::wants_input())
		{
			return false;
		}
#endif

		switch ((SDL_MouseButtonFlags)_mouseButtonState)
		{
		case SDL_BUTTON_LEFT:
		{
			return mouse == EMouse::Left;
			break;
		}

		case SDL_BUTTON_MIDDLE:
		{
			return mouse == EMouse::Middle;
		}

		case SDL_BUTTON_RIGHT:
		{
			return mouse == EMouse::Right;
		}

		case SDL_BUTTON_X1:
		{
			return mouse == EMouse::X1;
		}

		case SDL_BUTTON_X2:
		{
			return mouse == EMouse::X2;
		}

		default:
		{
			return false;
		}
		}
	}

	b8 WindowSDL::key_down(EKey const& key) const
	{
#ifdef NIGHT_ENABLE_DEBUG_RENDERER
		if (DebugRenderer::wants_input())
		{
			return false;
		}
#endif

		SDL_Scancode code = SDL_SCANCODE_UNKNOWN;
		for (const auto& i : sdl_scancode_map)
		{
			if (i.second == key)
			{
				code = i.first;
			}
		}

		if (code != SDL_SCANCODE_UNKNOWN)
		{
			int keyboard_state_length;
			bool const* keyboard_state = SDL_GetKeyboardState(&keyboard_state_length);
			ASSERT(code < keyboard_state_length);
			return keyboard_state[code];
		}

		return false;
	}

	pair<IRenderer*, RendererParams> WindowSDL::create_renderer()
	{
		RendererParams params;
		params.window = this;
		params.event_callback = event_callback();
		return { new RendererOpenGL, params };
	}

	SDL_Color WindowSDL::sdl_color(const Color& color) 
	{
		return { (b8)(color.r * 255), (b8)(color.g * 255), (b8)(color.b * 255), (b8)(color.a * 255) };
	}

	fvec2 WindowSDL::local_to_internal(const vec2& coordinate) const // TODO: handle aspect ratio
	{
		s32 w = width();
		s32 h = height();

		fvec2 result = { (r32)coordinate.x, -(r32)coordinate.y };
		//result.x *= h < w ? (r32)h / (r32)w : 1.0f;
		//result.y *= w < h ? (r32)w / (r32)h : 1.0f;
		result.x += 1.0f;
		result.y += 1.0f;
		result.x *= w / 2;
		result.y *= h / 2;
		//result.y = -result.y; // TODO: test this.
		return result;
	}

	vec2 WindowSDL::internal_to_local(const ivec2& coordinate) const
	{
		s32 w = width();
		s32 h = height();

		vec2 result = { (real)coordinate.x, (real)coordinate.y };
		result.x /= (real)w / 2;
		result.y /= (real)h / 2;
		result.x -= 1.0f;
		result.y -= 1.0f;
		//result.x /= h < w ? (real)h / (real)w : 1.0f;
		//result.y /= w < h ? (real)w / (real)h : 1.0f;
		//result.y = -result.y; // up is +.
		return result;
	}

	void WindowSDL::cursor_visibility(b8 visibility)
	{
		Application::get().queue_for_main_thread([this, visibility]()
			{
				if (visibility && !this->cursor_visibility())
				{
					if (!SDL_ShowCursor())
					{
						ERROR("SDL failed to show cursor, SDL_ERROR: {0}", SDL_GetError());
					}

					this->_isCursorVisible = SDL_CursorVisible();
				}
				else if (this->cursor_visibility())
				{
					if (!SDL_HideCursor())
					{
						ERROR("SDL failed to show cursor, SDL_ERROR: {0}", SDL_GetError());
					}

					this->_isCursorVisible = SDL_CursorVisible();
				}
			});

		//_isCursorVisible = visibility;
	}

	b8 WindowSDL::cursor_visibility() const
	{
		//return SDL_CursorVisible();
		return _isCursorVisible;
	}

	void WindowSDL::fullscreen(b8 make_fullscreen)
	{
		Application::get().queue_for_main_thread([this, make_fullscreen]()
			{
				if (!SDL_SetWindowFullscreen(this->_sdlWindow, (bool)make_fullscreen))
				{
					ERROR("SDL failed to set window fullscreen, SDL_ERROR: {0}", SDL_GetError());
				}

				_isFullscreen = make_fullscreen;
			});
	}

	b8 WindowSDL::fullscreen() const
	{
		return _isFullscreen;
	}

	void WindowSDL::grab_mouse()
	{
		Application::get().queue_for_main_thread([this]()
			{
				ASSERT(this->_sdlWindow != nullptr);
				if(!SDL_SetWindowRelativeMouseMode(this->_sdlWindow, true))
				{
					WARNING("WindowSDL failed to grab mouse, SDL_ERROR: {0}", SDL_GetError());
				}

				this->_isMouseGrabbed = SDL_GetWindowRelativeMouseMode(this->_sdlWindow);

				//if (this->_isMouseGrabbed)
				//{
				//	if (!SDL_ShowCursor())
				//	{
				//		WARNING("WindowSDL failed to hide mouse, SDL_ERROR: {0}", SDL_GetError());
				//	}
				//
				//	this->_mousePositionBeforeGrabbing = this->_mousePosition;
				//}
				//else
				//{
				//	if (!SDL_HideCursor())
				//	{
				//		WARNING("WindowSDL failed to hide mouse, SDL_ERROR: {0}", SDL_GetError());
				//	}
				//
				//	SDL_WarpMouseInWindow(this->_sdlWindow, this->_mousePositionBeforeGrabbing.x, this->height() - this->_mousePositionBeforeGrabbing.y);
				//}
			});
	}

	void WindowSDL::release_mouse()
	{
		Application::get().queue_for_main_thread([this]()
			{
				ASSERT(this->_sdlWindow != nullptr);
				if (!SDL_SetWindowRelativeMouseMode(this->_sdlWindow, false))
				{
					WARNING("WindowSDL failed to grab mouse, SDL_ERROR: {0}", SDL_GetError());
				}

				this->_isMouseGrabbed = SDL_GetWindowRelativeMouseMode(this->_sdlWindow);

				//if (this->_isMouseGrabbed)
				//{
				//	if (!SDL_ShowCursor())
				//	{
				//		WARNING("WindowSDL failed to hide mouse, SDL_ERROR: {0}", SDL_GetError());
				//	}
				//	this->_mousePositionBeforeGrabbing = this->_mousePosition;
				//}
				//else
				//{
				//	if (!SDL_HideCursor())
				//	{
				//		WARNING("WindowSDL failed to hide mouse, SDL_ERROR: {0}", SDL_GetError());
				//	}
				//
				//	SDL_WarpMouseInWindow(this->_sdlWindow, this->_mousePositionBeforeGrabbing.x, this->height() - this->_mousePositionBeforeGrabbing.y);
				//}
			});
	}

	b8 WindowSDL::is_mouse_grabbed()
	{
		return _isMouseGrabbed;
	}

	void WindowSDL::warp_mouse(vec2 window_position)
	{
		Application::get().queue_for_main_thread([this, window_position]()
			{
				vec2 global = local_to_internal(window_position);
				SDL_WarpMouseInWindow(this->_sdlWindow, (float)global.x, (float)global.y);
			});
	}

	real WindowSDL::precise_time_elapsed()
	{
		return ((real)(SDL_GetTicks() - _startTick) / 1000.0f);
	}

}