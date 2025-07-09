#pragma once

#include "IShader.h"
//#include "texture/ITexture.h"
#include "ref/ref.h"
#include "handle/handle.h"
#include "resource_manager/IResource.h"
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

	struct TextureUniformData
	{
		shandle<const ITexture> texture{ nullptr };
		u8 sample_depth_buffer{ false };

		u8 operator<(TextureUniformData const& rhs) const
		{
			return (texture.get() < rhs.texture.get() || sample_depth_buffer < rhs.sample_depth_buffer);
		}
	};

	struct NIGHT_API IMaterial : public IResource
	{
		// TODO: load material file
		IMaterial(MaterialParams const& params, string const& id);

		virtual void init() = 0;
		virtual void __tempInitHandle() override;
		virtual void clean() = 0;

		handle<IShader> shader() { return _shader; }

		template<typename T>
		u8 uniform(const string& name, const T& value);

		//virtual u8 uniform(const string& name, handle<ITexture> texture) = 0;
		//virtual u8 uniform(string const& name, DepthBuffer depth_buffer) = 0;

		virtual u8 uniform(const string& name, TextureUniformData const& texture) = 0;

		u8 has_uniform(string const& name) const
		{
			auto f = _uniforms.find(name);
			return f != _uniforms.end();
		}

		virtual void clear_uniforms() = 0;

		//u8 should_use_depth_peeling{ true };
		u8 should_use_depth_peeling() const;

		static void update_materials();

	protected:

		handle<IShader> _shader;
		vector<u8> _data; // TODO: make some kind of material instance struct
		umap<string, Uniform> _uniforms;

	private:

		static set<handle<IShader>> _toBeInitialized;
	};

	template<typename T>
	u8 IMaterial::uniform(const string& name, const T& value)
	{
		auto i = _uniforms.find(name);
		if (i != _uniforms.end())
		{
			auto& uniform = (*i).second;

			// TODO: check type and size.

			if (uniform.index + sizeof(T) > _data.size())
			{
				WARNING("data out of bounds, did you forget to use the full uniform in the shader?, name: %s", name.c_str());
				return false;
			}

			memcpy(&_data[uniform.index], &value, sizeof(T));

			return true;
		}

		WARNING("uniform not found!, name: %s", name.c_str());
		return false;
	}

}