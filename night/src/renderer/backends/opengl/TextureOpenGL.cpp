
#include "nightpch.h"
#include "TextureOpenGL.h"
#include "GL/glew.h"
#include "log/log.h"
#include "GLCall.h"
#include "texture/Surface.h"

namespace night
{

#if 0
	TextureOpenGL::TextureOpenGL(TextureParams const& params)
		: _id(0)
	{
		auto fn = [&](Surface const& surface) -> s32
		{
			// frame buffer
			GLCall(glGenFramebuffers(1, &_fbo));

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				ERROR("Texture frame buffer failed");
				return -1;
			}

			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));

			// texture
			GLCall(glGenTextures(1, &_id));
			GLCall(glBindTexture(GL_TEXTURE_2D, _id));

			switch (params.filtering)
			{
			case ETextureFiltering::Nearest:
			{
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
				break;
			}

			case ETextureFiltering::Linear:
			{
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				break;
			}

			default:
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				break;
			}

			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface.width(), surface.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, surface.pixels()));
			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _id, 0));
			
			// depth buffer
			GLCall(glGenTextures(1, &_dbo));
			GLCall(glBindTexture(GL_TEXTURE_2D, _dbo));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, surface.width(), surface.height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _dbo, 0));

			GLCall(glClear(GL_DEPTH_BUFFER_BIT));

			//width(surface.width());
			//height(surface.height());
			size({ surface.width(), surface.height() });

			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			//TRACE("Texture created, id: ", _id, ", fbo: ", _fbo, ", dbo: ", _dbo);

			return 1;
		};
		if (params.surface != nullptr)
		{
			fn(*params.surface);
		}
		else if (!params.path.empty())
		{
			Surface surface(SurfaceParams{.path = params.path});
			fn(surface);
		}
		else
		{
			ERROR("Texture params contain neither surface nor path");
			return;
		}
	}
#endif

	s32 TextureOpenGL::init()
	{
		ASSERT(_surfaceToBeInitialized != nullptr);

		// frame buffer
		GLCall(glGenFramebuffers(1, &_fbo));

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			ERROR("Texture frame buffer failed");
			return -1;
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));

		// texture
		GLCall(glGenTextures(1, &_id));
		GLCall(glBindTexture(GL_TEXTURE_2D, _id));

		switch (_filtering)
		{
		case ETextureFiltering::Nearest:
		{
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			break;
		}

		case ETextureFiltering::Linear:
		{
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			break;
		}

		default:
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			break;
		}

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _surfaceToBeInitialized->pixels()));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _id, 0));

		// depth buffer
		GLCall(glGenTextures(1, &_dbo));
		GLCall(glBindTexture(GL_TEXTURE_2D, _dbo));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _dbo, 0));

		GLCall(glClear(GL_DEPTH_BUFFER_BIT));

		/*size({ _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height() });*/

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		TRACE("Texture created, id: ", _id, ", fbo: ", _fbo, ", dbo: ", _dbo);

		return 1;
	}

	void TextureOpenGL::clean()
	{
		ASSERT(_id != 0);
		ASSERT(_dbo != 0);
		ASSERT(_fbo != 0);

		TRACE("Destroying Texture, id: ", _id);
		GLCall(glDeleteFramebuffers(1, &_fbo));
		GLCall(glDeleteTextures(1, &_dbo));
		GLCall(glDeleteTextures(1, &_id));
	}

	//TextureOpenGL::~TextureOpenGL()
	//{
	//	ASSERT(_id != 0);
	//	ASSERT(_dbo != 0);
	//	ASSERT(_fbo != 0);

	//	//TRACE("Destroying Texture, id: ", _id);
	//	GLCall(glDeleteFramebuffers(1, &_fbo));
	//	GLCall(glDeleteTextures(1, &_dbo));
	//	GLCall(glDeleteTextures(1, &_id));
	//}

	void TextureOpenGL::bind_fbo() const
	{
		ASSERT(_fbo != 0);
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
		GLCall(glViewport(0, 0, width(), height()));
	}

	// TODO: clear color is not working properly
	void TextureOpenGL::on_clear()
	{
		ASSERT(_pendingClear != Color(-1, -1, -1, -1));

		if (_pendingClear != Color(-1, -1, -1, -1))
		{
			ASSERT(_fbo != 0);
			//Color8 color8(clear_color);
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
			GLCall(glClearColor(_pendingClear.r, _pendingClear.g, _pendingClear.b, _pendingClear.a));
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}
	}

	void TextureOpenGL::on_resize()
	{
		ASSERT(_pendingResize != ivec2(0));
		if (_pendingResize != ivec2(0))
		{
			ASSERT(_id != 0);
			ASSERT(_dbo != 0);

			GLCall(glBindTexture(GL_TEXTURE_2D, _id));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _pendingResize.x, _pendingResize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

			GLCall(glBindTexture(GL_TEXTURE_2D, _dbo));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _pendingResize.x, _pendingResize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));

			// have to clear depth buffer for some reason
			ASSERT(_fbo != 0);
			//Color8 color8(clear_color);
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
			GLCall(glClearColor(0, 0, 0, 0));
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}
	}

}