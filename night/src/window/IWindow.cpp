
#include "nightpch.h"
#include "IWindow.h"
#include "application/Application.h"
#include "renderer/IRenderer.h"
#include "event/ApplicationEvent.h"
#include "utility.h"

namespace night
{
	IWindow::IWindow(const WindowParams& params)
	{
		if (params.width == -1 || params.height == -1)
		{
			auto& res = utility::settings()["Night"]["Window"]["Resolution"];
			if (res.empty())
			{
				res.set_vector(_width, _height);
				//_width = NIGHT_WINDOW_DEFAULT_WIDTH;
				//_height = NIGHT_WINDOW_DEFAULT_HEIGHT;
			}
			else
			{
				res.get_vector(_width, _height);
			}
		}
		else
		{
			_width = params.width;
			_height = params.height;
		}

		if (params.fps == -1)
		{
			auto& fps = utility::settings()["Night"]["Window"]["FPS"];

			if (fps.empty())
			{
				fps.set_real(_fps);
				//_fps = NIGHT_WINDOW_DEFAULT_FPS;
			}
			else
			{
				_fps = fps.get_real();
			}
		}
		else
		{
			_fps = params.fps;
		}

		_title = params.title;

		_eventCallback = params.eventCallback;
		_timeElapsed = 0.0f;
		_deltaTime = _fps / 1000.0f;
	}

	s32 IWindow::init()
	{
		auto [renderer, rparams] = create_renderer();
		_renderer = renderer;
		return _renderer->init(rparams);
	}

	void IWindow::close()
	{
		if (_renderer)
		{
			_renderer->close();
			delete _renderer;
			_renderer = nullptr;
		}
		
		TRACE("Destroying Window");
	}

	real IWindow::aspect_ratio() const
	{
		real a = (real)_height / (real)_width;
		real b = (real)_width / (real)_height;
		return a < b ? a : b;
	}

	u8 IWindow::on_resize(WindowResizeEvent& event)
	{
		_width = event.width();
		_height = event.height();

		return true;
	}

}