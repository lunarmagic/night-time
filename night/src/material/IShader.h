#pragma once

//#include "stl/stl.h"
#include "math/math.h"
#include "resource_manager/IResource.h"

namespace night
{

	struct ShaderParams
	{
		string path;
		string mem_vs;
		string mem_fs;
		//u8 load_from_memory{ false };
	};

	struct NIGHT_API IShader : public IResource
	{
		IShader(ShaderParams const& params, string const& id);
		virtual void init() = 0;

		virtual void __tempInitHandle() override;

		virtual void clean() = 0;

		// TODO: dispatch these in main thread
		virtual void uniform1i(const string& name, s32 value) = 0;
		virtual void uniform1f(const string& name, real value) = 0;
		virtual void uniform2f(const string& name, real v0, real v1) = 0;
		virtual void uniform3f(const string& name, real v0, real v1, real v2) = 0;
		virtual void uniform4f(const string& name, real v0, real v1, real v2, real v3) = 0;
		virtual void uniformMat4f(const string& name, const mat4& matrix) = 0;

		static void update_shaders();

	protected:

		ShaderParams _params{};

		static set<handle<IShader>> _toBeInitialized;

		virtual u8 load_from_path(string const& path) = 0;
		virtual u8 load_from_memory(const string& vs, const string& fs) = 0;
	};

}