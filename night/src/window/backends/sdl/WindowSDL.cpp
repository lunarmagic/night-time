
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

				case SDL_EVENT_MOUSE_MOTION:
				{
					if (!gui_result.wants_mouse_capture) // may not want this
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

						MouseMotionEvent e(local);
						callback(e);
					}
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

				case SDL_EVENT_WINDOW_RESIZED:
				{
					WindowResizeEvent e(event.window.data1, event.window.data2);
					renderer().on_window_resize(event.window.data1, event.window.data2);
					on_resize(e); // TODO: may want to call this from application.
					callback(e);
					break;
				}
				}
			}
		}
	}

	void WindowSDL::update() // TODO: map sdl events to our own events
	{
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
		if (std::_Is_nan(lerp(_framerate, (real)fps_delta_time, 0.01f)))
		{
			_framerate = (real)fps_delta_time;
		}
		else
		{
			_framerate = lerp(_framerate, (real)fps_delta_time, 0.01f);
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

	//ref<ITexture> WindowSDL::create_texture(const string& id, const TextureParams& params)
	//{
	//	auto i = find_texture(id);
	//	if (i != nullptr)
	//	{
	//		WARNING("Texture already created!, id: ", id);
	//		return i;
	//	}

	//	sref<WindowSDL::Texture> texture(new WindowSDL::Texture(this, params));
	//	textures()[id] = (sref<ITexture>)texture;

	//	TRACE("Window created texture, id: ", id);

	//	return (ref<ITexture>)texture;
	//}

	vec2 WindowSDL::mouse() const
	{
		r32 x;
		r32 y;
		SDL_GetMouseState(&x, &y);

		y = height() - y; // flip coord for opengl

		return internal_to_local({ x, y }); // TODO: pass in r32s
	}

	u8 WindowSDL::key_down(EKey const& key) const
	{
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

	//ref<ISurface> WindowSDL::create_surface(const string& id, const SurfaceParams& params)
	//{
	//	auto i = find_surface(id);
	//	if (i != nullptr)
	//	{
	//		WARNING("Texture already created!, id: ", id);
	//		return i;
	//	}

	//	sref<WindowSDL::Surface> surface(new WindowSDL::Surface(params));
	//	surfaces()[id] = (sref<ISurface>)surface;

	//	TRACE("Window created texture, id: ", id);

	//	return (ref<ISurface>)surface;
	//}

	SDL_Color WindowSDL::sdl_color(const Color& color) 
	{
		return { (u8)(color.r * 255), (u8)(color.g * 255), (u8)(color.b * 255), (u8)(color.a * 255) };
	}

	fvec2 WindowSDL::local_to_internal(const vec2& coordinate) const // TODO: handle aspect ratio
	{
		s32 w = width();
		s32 h = height();

		fvec2 result = { (r32)coordinate.x, (r32)coordinate.y };
		//result.x *= h < w ? (r32)h / (r32)w : 1.0f;
		//result.y *= w < h ? (r32)w / (r32)h : 1.0f;
		result.x += 1.0f;
		result.y += 1.0f;
		result.x *= w / 2;
		result.y *= h / 2;
		//result.y = -result.y; // TODO: test this.
		return result;
	}

	//fvec2 WindowSDL::local_to_internal(const fvec2& coordinate) const
	//{
	//	s32 w = width();
	//	s32 h = height();
	//
	//	fvec2 result = { coordinate.x, coordinate.y };
	//	result.x *= h < w ? (r32)h / (r32)w : 1.0f;
	//	result.y *= w < h ? (r32)w / (r32)h : 1.0f;
	//	result.x += 1.0f;
	//	result.y += 1.0f;
	//	result.x *= w / 2;
	//	result.y *= h / 2;
	//	return result;
	//}

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

	void WindowSDL::cursor_visibility(u8 visibility)
	{
		if (visibility && !cursor_visibility())
		{
			if (!SDL_ShowCursor())
			{
				ERROR("SDL failed to show cursor, SDL_ERROR: ", SDL_GetError());
			}
		}
		else if(cursor_visibility())
		{
			if (!SDL_HideCursor())
			{
				ERROR("SDL failed to show cursor, SDL_ERROR: ", SDL_GetError());
			}
		}
	}

	u8 WindowSDL::cursor_visibility() const
	{
		return SDL_CursorVisible();
	}

	void WindowSDL::fullscreen(u8 make_fullscreen)
	{
		if (!SDL_SetWindowFullscreen(_sdlWindow, (bool)make_fullscreen))
		{
			ERROR("SDL failed to set window fullscreen, SDL_ERROR: ", SDL_GetError());
		}

		_isFullscreen = make_fullscreen;
	}

	u8 WindowSDL::fullscreen() const
	{
		return _isFullscreen;
	}

	real WindowSDL::precise_time_elapsed()
	{
		return ((real)(SDL_GetTicks() - _startTick) / 1000.0f);
	}

	//WindowSDL::Texture::Texture(WindowSDL* const window, const TextureParams& params)
	//	: ITexture(params)
	//{
	//	//ASSERT(window != nullptr);

	//	//_sdlWindow = window;

	//	//if (!params.path.empty())
	//	//{
	//	//	SDL_Surface* sdl_surface = IMG_Load(params.path.c_str());
	//	//	
	//	//	if (!sdl_surface)
	//	//	{
	//	//		WARNING("SDL_Image failed to load image, path: ", params.path);
	//	//	}
	//	//	
	//	//	_sdlTexture = SDL_CreateTextureFromSurface(((RendererSDL&)window->renderer()).sdl_renderer(), sdl_surface);
	//	//	
	//	//	if (!_sdlTexture)
	//	//	{
	//	//		WARNING("SDL failed to create texture from surface!, SDL_error: ", SDL_GetError());
	//	//		return;
	//	//	}
	//	//}
	//	//else if (params.surface != nullptr) // TODO: add error message
	//	//{
	//	//	ref<WindowSDL::Surface> surface;
	//	//	surface = params.surface;

	//	//	_sdlTexture = SDL_CreateTextureFromSurface(((RendererSDL&)window->renderer()).sdl_renderer(), surface->sdl_surface());

	//	//	if (!_sdlTexture)
	//	//	{
	//	//		WARNING("SDL failed to create texture from surface!, SDL_error: ", SDL_GetError());
	//	//		return;
	//	//	}
	//	//}
	//	//else
	//	//{
	//	//	_sdlTexture = SDL_CreateTexture(((RendererSDL&)window->renderer()).sdl_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, params.width, params.height);

	//	//	if (!_sdlTexture)
	//	//	{
	//	//		WARNING("SDL failed to create texture!, SDL_error: ", SDL_GetError());
	//	//		return;
	//	//	}
	//	//}

	//	////s32 w, h;

	//	////TextureParams new_params = {};
	//	//
	//	//r32 w = 0.0f;
	//	//r32 h = 0.0f;
	//	//u8 error = SDL_GetTextureSize(_sdlTexture, &w, &h);
	//	////new_params.width = (s32)w;
	//	////new_params.height = (s32)h;

	//	//if (!error)
	//	//{
	//	//	ERROR("SDL_QueryTexture failed!, SDL_Error: ", SDL_GetError());
	//	//}

	//	//width((s32)w);
	//	//height((s32)h);

	//	////ITexture::ITexture(new_params);

	//	////width(w);
	//	////height(h);
	//}

	//WindowSDL::Texture::~Texture()
	//{
	//	if (_sdlTexture)
	//	{
	//		SDL_DestroyTexture(_sdlTexture);
	//	}
	//}

	//WindowSDL::Surface::Surface(const SurfaceParams& params)
	//	: ISurface(params)
	//{
	//	if (params.path.empty())
	//	{
	//		// create blank surface
	//		//_sdlSurface = SDL_CreateRGBSurface(
	//		//	0,
	//		//	width(),
	//		//	height(),
	//		//	32,
	//		//	0x000000FF,
	//		//	0x0000FF00,
	//		//	0x00FF0000,
	//		//	0xFF000000
	//		//);

	//		_sdlSurface = SDL_CreateSurface(width(), height(), SDL_PIXELFORMAT_RGBA8888);
	//		//SDL_Palette* palette = SDL_CreateSurfacePalette(_sdlSurface); // TODO: figure out what this is

	//		if (!_sdlSurface)
	//		{
	//			WARNING("SDL failed to create surface! SDL_Error: ", SDL_GetError());
	//		}
	//	}
	//	else
	//	{
	//		_sdlSurface = IMG_Load(params.path.c_str());

	//		if (!_sdlSurface)
	//		{
	//			WARNING("SDL_Image failed to load image, path: ", params.path);
	//		}

	//		width(_sdlSurface->w);
	//		height(_sdlSurface->h);
	//	}
	//}

	//WindowSDL::Surface::~Surface()
	//{
	//	if (_sdlSurface)
	//	{
	//		SDL_DestroySurface(_sdlSurface);
	//	}
	//}

	//Color8* WindowSDL::Surface::pixels() const
	//{
	//	if (_sdlSurface)
	//	{
	//		return (Color8*)_sdlSurface->pixels;
	//	}

	//	WARNING("SDL surface is nullptr!");
	//	return nullptr;
	//}

}