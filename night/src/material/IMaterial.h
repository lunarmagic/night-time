#pragma once

#include "IShader.h"
#include "ref/ref.h"
#include "handle/handle.h"
#include "resource_manager/IResource.h"
#include "texture/TextureUniformData.h"
#include "log/log.h"

namespace night
{

	struct ITexture;
	struct DepthBuffer;

	struct MaterialParams
	{
		handle<IShader> shader;
	};

	struct Uniform
	{
		u32 type{ 0x1406 }; // TODO: make enum
		s32 size{ 0 };
		u32 index{ 0 };
	};

	enum class EMaterialDepthFunction : s32
	{
		Less = 0,
		Greater
	};

	/*
		WARNING: if passing struct as uniform, the reals within the struct
		are not automatically converted into gl_reals, you have to manually
		keep track of the type within the shader and make sure
		it matches that of night::real, double or float
		depending on if NIGHT_USE_DOUBLE_PRECISION is defined.
	*/
	struct NIGHT_API IMaterial : public IResource
	{
		// TODO: load material file
		IMaterial(MaterialParams const& params, string const& id);

		virtual void init() = 0;
		virtual void __tempInitHandle() override;
		virtual void clean() = 0;

		handle<IShader> shader() { return _shader; }

		template<typename T>
		b8 uniform(const string& name, const T& value);

		virtual b8 uniform(const string& name, TextureUniformData const& texture) = 0;

		b8 has_uniform(string const& name) const
		{
			auto f = _uniforms.find(name);
			return f != _uniforms.end();
		}

		virtual void clear_uniforms() = 0;

		b8 should_use_depth_peeling() const;

	protected:

		handle<IShader> _shader;
		vector<u8> _data;
		umap<string, Uniform> _uniforms;
	};

	template<typename T>
	b8 IMaterial::uniform(const string& name, const T& value)
	{
		auto i = _uniforms.find(name);
		if (i != _uniforms.end())
		{
			auto& uniform = (*i).second;

			// TODO: check type and size.

			if (uniform.index + sizeof(T) > _data.size())
			{
				WARNING("data out of bounds, name: {0}", name);
				return false;
			}

			memcpy(&_data[uniform.index], &value, sizeof(T));

			return true;
		}

		WARNING("uniform not found!, name: {0}", name);
		return false;
	}

}