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
	struct Quad;
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
		virtual u8 key_down(EKey const& key) const override;

		SDL_Window* sdl_window() const { return _sdlWindow; }

		fvec2 local_to_internal(const vec2& coordinate) const;
		//fvec2 local_to_internal(const fvec2& coordinate) const;
		vec2 internal_to_local(const ivec2& coordinate) const;

		virtual void cursor_visibility(u8 visibility) override;
		virtual u8 cursor_visibility() const override;

		virtual void fullscreen(u8 make_fullscreen) override;
		virtual u8 fullscreen() const override;

		virtual real precise_time_elapsed() override;

	protected:

		virtual pair<IRenderer*, RendererParams> create_renderer() override;

	private:

		SDL_Window* _sdlWindow{ nullptr };

		u64 _frameTick;
		u64 _startTick;
		u8 _isFullscreen{ false };
	};

}