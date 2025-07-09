#pragma once

#include "texture/ITexture.h"

namespace night
{

	struct TextureOpenGL : public ITexture
	{
		TextureOpenGL(TextureParams const& params, string const& id) : ITexture(params, id) {}
		//TextureOpenGL(TextureParams const& params);
		//~TextureOpenGL();
		virtual s32 init();
		virtual void clean() override;

		u32 const& id() const { return _id; }
		u32 const& fbo() const { return _fbo; }
		u32 const& dbo() const { return _dbo; }

		//void bind_fbo() const;
		void bind_fbo() const;

		virtual void on_clear() override;

		virtual void on_resize() override;

	private:

		u32 _id{ 0 };
		u32 _fbo{ 0 };
		u32 _dbo{ 0 };
		s32 _channels{ 0 };
	};

}