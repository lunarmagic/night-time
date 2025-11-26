
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

	s32 utility::setting(initializer_list<string> const& path, s32 def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_s32(def);
			return def;
		}

		return branch.get_s32();

		//return branch.empty() ? def : branch.get_s32();
	}

	real utility::setting(initializer_list<string> const& path, real def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_real(def);
			return def;
		}

		return branch.get_real();
		//return branch.empty() ? def : branch.get_real();
	}

	string utility::setting(initializer_list<string> const& path, string const& def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_string(def);
			return def;
		}

		return branch.get_string();
	}

	vec2 utility::setting(initializer_list<string> const& path, vec2 def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_vector(def.x, def.y);
			return def;
		}

		vec2 result = {};
		branch.get_vector(result.x, result.y);
		return result;
	}

	vec3 utility::setting(initializer_list<string> const& path, vec3 def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_vector(def.x, def.y, def.z);
			return def;
		}

		vec3 result = {};
		branch.get_vector(result.x, result.y, result.z);
		return result;
	}

	vec4 utility::setting(initializer_list<string> const& path, vec4 def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_vector(def.x, def.y, def.z, def.w);
			return def;
		}

		vec4 result = {};
		branch.get_vector(result.x, result.y, result.z, result.w);
		return result;
	}

	mat4 utility::setting(initializer_list<string> const& path, mat4 def)
	{
		//File& branch = utility::settings()[path];

		//if (branch.empty())
		//{
		//	return def;
		//}
		//else
		//{
		//	mat4 result = {};
		//	branch.get_vector(
		//		result[0], result[1], result[2], result[3],
		//		result[4], result[5], result[6], result[7],
		//		result[8], result[9], result[10], result[11],
		//		result[12], result[13], result[14], result[15]);
		//	return result;
		//}
		ASSERT(false); // TODO: implement
		return {};
	}

	quat utility::setting(initializer_list<string> const& path, quat def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_vector(def.x, def.y, def.z, def.w);
			return def;
		}

		quat result = {};
		branch.get_vector(result.x, result.y, result.z, result.w);
		return result;
	}

	Color utility::setting(initializer_list<string> const& path, Color def)
	{
		File& branch = utility::settings()[path];

		if (branch.empty())
		{
			branch.set_vector(def.r, def.g, def.b, def.a);
			return def;
		}
		
		Color result = {};
		branch.get_vector(result.r, result.g, result.b, result.a);
		return result;
	}

	umultimap<string, InputKey> const& utility::action_map()
	{
		return Application::get().action_map;
	}

	b8 utility::key_down(EKey const& key)
	{
		return Application::get().window().key_down(key);
	}

	b8 utility::mouse_down(EMouse const& mouse)
	{
		return Application::get().window().mouse_down(mouse);
	}
}


