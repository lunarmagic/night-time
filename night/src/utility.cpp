
#include "nightpch.h"
#include "utility.h"
#include "application/Application.h"

namespace night
{

	IWindow& utility::window()
	{
		return Application::get().window();
	}

	IRenderer& utility::renderer()
	{
		return Application::get().window().renderer();
	}

	IResourceManager& utility::resource_manager()
	{
		return Application::get().resource_manager();
	}

	IGui& utility::gui()
	{
		return Application::get().window().gui();
	}

	File& utility::settings()
	{
		return Application::get().settings;
	}

	umultimap<string, InputKey> const& utility::action_map()
	{
		return Application::get().action_map;
	}

	u8 utility::key_down(EKey const& key)
	{
		return Application::get().window().key_down(key);
	}
}


