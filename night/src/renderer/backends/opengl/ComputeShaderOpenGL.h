#pragma once

#include "material/IComputeShader.h"
#include "math/math.h"
#include "ShaderStorageBufferOpenGL.h"

namespace night
{

	struct ComputeShaderOpenGL : public IComputeShader
	{
		ComputeShaderOpenGL(ComputeShaderParams const& params, string const& id) : IComputeShader(params, id) {}

		virtual void init() override;
		virtual void clean() override;

		//virtual void reserve(s32 size, s32 binding = 0) override;

		virtual void data(void* data, size_t size, s32 binding = 0) override;

		virtual void data(void* out_data, s32 binding = 0) override;

		virtual void uniform1i(const string& name, s32 value) override;
		virtual void uniform1f(const string& name, real value) override;
		virtual void uniform2f(const string& name, real v0, real v1) override;
		virtual void uniform3f(const string& name, real v0, real v1, real v2) override;
		virtual void uniform4f(const string& name, real v0, real v1, real v2, real v3) override;
		virtual void uniformMat4f(const string& name, const mat4& matrix) override;

		virtual void compute() const override;


		// TODO: add hot-reloading

		u32 id() const { return _rendererID; }

		s32 uniform_location(const string& name);

	protected:

		virtual u8 load_from_path(const string& path) override;
		virtual u8 load_from_memory(const string& str) override;

	private:

		umap<s32, ShaderStorageBufferOpenGL> _ssbos;

		// TODO: add map of ssbos
		u32 _rendererID{ 0 };
		umap<string, s32> _uniformLocationCache;
	};

}

