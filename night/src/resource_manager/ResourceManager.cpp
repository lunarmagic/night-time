
#include "nightpch.h"
#include "ResourceManager.h"
#include "texture/ITexture.h"
#include "thread/Thread.h"
#include "material/IComputeShader.h"

#include "utility.h"

namespace night
{

#ifdef false
	u8 IResourceManager::add_directory(string const& path)
	{
		if (!filesystem::exists(path))
		{
			WARNING("directory does not exist!");
			return false;
		}

		if (!filesystem::is_directory(path))
		{
			WARNING("directory is not a directory!");
			return false;
		}

		//auto it = filesystem::directory_iterator(directory);
		auto f = _directories.find(path);
		if (f == _directories.end())
		{
			WARNING("directory is already inside resource manager!");
			return false;
		}

		_directories.insert(path);

		return true;
	}

	string IResourceManager::find_directory(string const& path)
	{
		auto f = _directories.find(path);
		if (f == _directories.end())
		{
			WARNING("directory is already inside resource manager!");
			return "";
		}

		return (*f);
	}

	void ResourceManager::add_directories()
	{
		u8 btextures = add_directory("resources/textures");
		u8 bmaterials = add_directory("resources/textures");
		u8 bsounds = add_directory("resources/textures");

		ASSERT(btextures && bmaterials && bsounds);
	}

	void ResourceManager::on_add_directories()
	{
		// override to add more directories
	}
#endif

	void ResourceManager::load_resources()
	{
		// load settings:
		//settings.read("settings/night_settings.txt");

		// load materials, textures and sounds.
		// create them in the renderer/audio manager

		//Thread<void> test_thread([&]()
		//	{
		string path_textures = "resources/textures";
		string path_shaders = "resources/shaders";

		if (filesystem::exists(path_textures) && filesystem::is_directory(path_textures))
		{
			for (const auto& entry : filesystem::directory_iterator(path_textures))
			{
				if (entry.path().extension().string() == ".png")
				{
					TextureParams params;
					params.path = entry.path().string();
					string id = entry.path().stem().string();
					utility::renderer().create_texture(id, params);

					TRACE("loaded texture, filename/id: ", entry.path().stem().string());
				}
				else
				{
					string path = entry.path().string();
					string stem = entry.path().stem().string();
					WARNING("unsupported file type is contained within the textures resource folder, path: ", path, ", stem: ", stem);
				}
			}
		}

		if (filesystem::exists(path_shaders) && filesystem::is_directory(path_shaders))
		{
			for (const auto& entry : filesystem::directory_iterator(path_shaders))
			{
				if (entry.path().extension().string() == ".shader")
				{
					ShaderParams params;
					params.path = entry.path().string();
					string id = entry.path().stem().string();
					utility::renderer().create_shader(id, params);

					TRACE("loaded shader, filename/id: ", entry.path().stem().string());
				}
				else if (entry.path().extension().string() == ".material")
				{
					ShaderParams params;
					params.path = entry.path().string();
					string id = entry.path().stem().string();
					auto shader = utility::renderer().create_shader(id, params);

					MaterialParams mparams;
					mparams.shader = shader;
					utility::renderer().create_material(id, mparams);

					TRACE("loaded material, filename/id: ", entry.path().stem().string());
				}
				else if (entry.path().extension().string() == ".compute_shader")
				{
					ComputeShaderParams params;
					params.path = entry.path().string();
					string id = entry.path().stem().string();
					utility::renderer().create_compute_shader(id, params);

					TRACE("loaded compute shader, filename/id: ", entry.path().stem().string());
				}
				else
				{
					string path = entry.path().string();
					string stem = entry.path().stem().string();
					WARNING("unsupported file type is contained within the shaders resource folder, path: ", path, ", stem: ", stem);
				}
			}
		}

		//if (filesystem::exists(path_materials) && filesystem::is_directory(path_materials))
		//{
		//	for (const auto& entry : filesystem::directory_iterator(path_materials))
		//	{
		//	}
		//}
		
		//utility::renderer().update_resources();
		on_load_resources();

		//	});
		//test_thread.wait();
	}

	void ResourceManager::on_load_resources()
	{

	}

	void ResourceManager::close()
	{
		//settings.write(settings.data());
		on_close();
	}

	void ResourceManager::on_close()
	{

	}

}