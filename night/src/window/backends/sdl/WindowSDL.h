#pragma once

#include "core.h"
//#include "utility.h"
//#include "application/Application.h"
#include "window/IWindow.h"

//#include "color/Color.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Color;

namespace night
{

	struct Event;
	struct Color;
	struct WindowResizeEvent;
	struct Color;

	struct NIGHT_API WindowSDL : public IWindow
	{
		WindowSDL(WindowParams const& params) : IWindow(params) {};
		virtual s32 init() override;
		virtual void close() override;

		virtual void poll_events() override;
		virtual void update() override;
		virtual void present() override;

		static SDL_Color sdl_color(const Color& color);

		virtual vec2 mouse() const override;
		virtual b8 mouse_down(EMouse mouse) const override;
		virtual b8 key_down(EKey const& key) const override;

		SDL_Window* sdl_window() const { return _sdlWindow; }

		fvec2 local_to_internal(const vec2& coordinate) const;
		//fvec2 local_to_internal(const fvec2& coordinate) const;
		vec2 internal_to_local(const ivec2& coordinate) const;

		virtual void cursor_visibility(b8 visibility) override;
		virtual b8 cursor_visibility() const override;

		virtual void fullscreen(b8 make_fullscreen) override;
		virtual b8 fullscreen() const override;

		//virtual void grab_mouse(b8 should_grab) override;
		//virtual b8 grab_mouse() override;

		virtual void grab_mouse() override;
		virtual void release_mouse() override;
		virtual b8 is_mouse_grabbed() override;

		virtual void warp_mouse(vec2 window_position) override;

		virtual vec2 mouse_motion() const override { return _mouseMotion; }

		virtual real precise_time_elapsed() override;

	protected:

		virtual pair<IRenderer*, RendererParams> create_renderer() override;

	private:

		SDL_Window* _sdlWindow{ nullptr };

		u64 _frameTick;
		u64 _startTick;
		b8 _isFullscreen{ false };
		b8 _isCursorVisible = true;
		b8 _isMouseGrabbed = false;

		vec2 _mousePosition = vec2(0);
		u32 _mouseButtonState = {};

		vec2 _mouseMotion = vec2(0);

		//vec2 _mousePositionBeforeGrabbing = vec2(0);
	};

}