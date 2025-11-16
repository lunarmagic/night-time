
#include "nightpch.h"
#include "ResourceManager.h"
#include "texture/ITexture.h"
#include "thread/Thread.h"
#include "material/IComputeShader.h"

#include "utility.h"

namespace night
{

	void ResourceManager::load_resources()
	{
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

					TRACE("loaded texture, filename/id: {0}", entry.path().stem().string());
				}
				else
				{
					string path = entry.path().string();
					string stem = entry.path().stem().string();
					WARNING("unsupported file type is contained within the textures resource folder, path: {0}, stem: {1}", path, stem);
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

					TRACE("loaded shader, filename/id: {0}", entry.path().stem().string());
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

					TRACE("loaded material, filename/id: {0}", entry.path().stem().string());
				}
				else if (entry.path().extension().string() == ".compute_shader")
				{
					ComputeShaderParams params;
					params.path = entry.path().string();
					string id = entry.path().stem().string();
					utility::renderer().create_compute_shader(id, params);

					TRACE("loaded compute shader, filename/id: {0}", entry.path().stem().string());
				}
				else
				{
					string path = entry.path().string();
					string stem = entry.path().stem().string();
					WARNING("unsupported file type is contained within the shaders resource folder, path: {0}, stem: {1}", path, stem);
				}
			}
		}

		on_load_resources();
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