#pragma once

//#include "stl/stl.h"
#include "log/log.h"
#include "material/IMaterial.h"

namespace night
{
	struct MaterialOpenGL : public IMaterial
	{
		MaterialOpenGL(MaterialParams const& params, string const& id) : IMaterial(params, id) {}
		//~MaterialOpenGL();

		virtual void init() override;
		virtual void clean() override;

		//virtual u8 uniform(const string& name, handle<ITexture> texture) override;
		//virtual u8 uniform(string const& name, DepthBuffer depth_buffer) override;

		virtual u8 uniform(const string& name, TextureUniformData const& texture) override;

		u8 bind();

		u32 storage() const { return _storage; }

	private:

		u32 _storage{ 0 };

		//virtual void shader(handle<IShader> shader) override;
		void query_uniforms();
		void query_storage();

		virtual void clear_uniforms() override;
	};
}