#pragma once

#include "core.h"
#include "math/math.h"
#include "input/InputKey.h"
//#include "application/Application.h"
//#include "texture/ITexture.h"
//#include "texture/ISurface.h"
#include "color/Color.h"
//#include "geometry/Quad.h"
//#include "event/ApplicationEvent.h"

#define NIGHT_WINDOW_DEFAULT_WIDTH 1600
#define NIGHT_WINDOW_DEFAULT_HEIGHT 1000
#define NIGHT_WINDOW_DEFAULT_FPS 120

namespace night
{
	struct IRenderer;
	struct RendererParams;
	struct IGui;
	struct Quad;
	struct Event;
	struct WindowResizeEvent;
	struct ITexture;
	struct WindowParams;

	struct NIGHT_API IWindow
	{
		//IWindow() = default;
		IWindow(const WindowParams& params);
		virtual s32 init();
		virtual void close();

		virtual void poll_events() = 0;
		virtual void update() = 0;
		virtual void present() = 0;
		//virtual void render() = 0;

		//virtual void draw_point(const vec2& point, const Color& color) const = 0;
		//virtual void draw_line(const vec2& p1, const vec2& p2, const Color& color) const = 0;
		//virtual void draw_quad(const Quad& quad, ref<ITexture> texture) const = 0;

		//virtual ref<ISurface> create_surface(const string& id, const SurfaceParams& params) = 0;
		//ref<ISurface> find_surface(const string& id) const;
		//void destroy_surface(const string& id);

		//virtual ref<ITexture> create_texture(const string& id, const TextureParams& params) = 0;
		//ref<ITexture> find_texture(const string& id) const;
		//void destroy_texture(const string& id);

		virtual vec2 mouse() const = 0;
		virtual u8 key_down(EKey const& key) const = 0;

		string const& title() const { return _title; }

		s32 width() const { return _width; }
		s32 height() const { return _height; }
		real aspect_ratio() const;

		real fps() const { return _fps; }
		void fps(real fps) { _fps = fps; }

		real delta_time() const { return _deltaTime; }
		r64 time_elapsed() const { return _timeElapsed; }

		virtual real precise_time_elapsed() = 0;

		virtual void cursor_visibility(u8 visibility) = 0;
		virtual u8 cursor_visibility() const = 0;

		virtual void fullscreen(u8 make_fullscreen) = 0;
		virtual u8 fullscreen() const = 0;

		//static IWindow& get();

		IRenderer& renderer() { return *_renderer; }
		IGui& gui() const { return *_gui; }
		real const& framerate() { return _framerate; }

	protected:

		IGui* _gui;
		real _framerate{ 0.0f };

		u8 on_resize(WindowResizeEvent& event);

		void delta_time(real dt) { _deltaTime = dt; }
		void time_elapsed(r64 te) { _timeElapsed = te; }

		const function<void(Event&)>& event_callback() const { return _eventCallback; }

		//map<string, sref<ISurface>>& surfaces() { return _surfaces; } // TODO: find better way
		//map<string, sref<ITexture>>& textures() { return _textures; } // TODO: find better way

		virtual pair<IRenderer*, RendererParams> create_renderer() = 0;
		
	private:

		IRenderer* _renderer;

		function<void(Event&)> _eventCallback{};
		s32 _width{ NIGHT_WINDOW_DEFAULT_WIDTH };
		s32 _height{ NIGHT_WINDOW_DEFAULT_HEIGHT };
		string _title;

		//map<string, sref<ISurface>> _surfaces;
		//map<string, sref<ITexture>> _textures;

		real _fps{ NIGHT_WINDOW_DEFAULT_FPS };
		real _deltaTime;
		r64 _timeElapsed;
	};

}