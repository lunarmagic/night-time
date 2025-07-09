#pragma once

#include "core.h"
#include "window/IWindow.h"
#include "renderer/IRenderer.h"
#include "resource_manager/ResourceManager.h"
#include "gui/Gui.h"

namespace night
{

	struct NIGHT_API utility
	{
		static IWindow& window();
		static IRenderer& renderer();
		static IResourceManager& resource_manager();
		static IGui& gui();

		static File& settings();
		static umultimap<string, InputKey> const& action_map();

		static u8 key_down(EKey const& key);
	};

}