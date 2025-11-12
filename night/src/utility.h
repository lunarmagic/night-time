#pragma once

#include "core.h"
#include "window/IWindow.h"
#include "renderer/IRenderer.h"
#include "resource_manager/ResourceManager.h"
#include "gui/Gui.h"

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define UNIQUE_VARIABLE_NAME(base) CONCAT(base, __COUNTER__)

namespace night
{

	struct NIGHT_API utility
	{
		static IWindow& window();
		static IRenderer& renderer();
		static IResourceManager& resource_manager();
		static IGui& gui();

		static File& settings();

		static s32 setting(initializer_list<string> const& path, s32 def = 0);
		static real setting(initializer_list<string> const& path, real def = 0);
		static string setting(initializer_list<string> const& path, string const& def = string{});
		static vec2 setting(initializer_list<string> const& path, vec2 def = vec2(0));
		static vec3 setting(initializer_list<string> const& path, vec3 def = vec3(0));
		static vec4 setting(initializer_list<string> const& path, vec4 def = vec4(0));
		static mat4 setting(initializer_list<string> const& path, mat4 def = mat4(1));
		static quat setting(initializer_list<string> const& path, quat def = quat(vec3(0)));
		static Color setting(initializer_list<string> const& path, Color def = COLOR_NULL);

		static umultimap<string, InputKey> const& action_map();

		static u8 key_down(EKey const& key);
		static u8 mouse_down(EMouse const& mouse);
	};

}