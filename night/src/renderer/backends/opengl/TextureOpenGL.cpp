
#include "nightpch.h"
#include "TextureOpenGL.h"
#include "GL/glew.h"
#include "log/log.h"
#include "GLCall.h"
#include "texture/Surface.h"

// TODO: for some reason if depth buffer is less than 513 in width and height, writing to the depth buffer leaks into unrelated depth buffers
#define NIGHT_OPENGL_TEXTURE_DEPTH_BUFFER_MIN_SIZE 513
//#define NIGHT_OPENGL_TEXTURE_DEPTH_BUFFER_MIN_SIZE 512 * 2 // TODO: change to 513

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

#if 0
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		GLCall(glGenTextures(1, &_id));
		GLCall(glBindTexture(GL_TEXTURE_2D, _id));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		//NULL means reserve texture memory, but texels are undefined
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL));
		GLCall(glGenTextures(1, &_dbo));
		GLCall(glBindTexture(GL_TEXTURE_2D, _dbo));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		//GLCall(glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
		//NULL means reserve texture memory, but texels are undefined
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL));
		//-------------------------
		GLCall(glGenFramebuffersEXT(1, &_fbo));
		GLCall(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo));
		//Attach 2D texture to this FBO
		GLCall(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _id, 0/*mipmap level*/));
		//-------------------------
		//Attach depth texture to FBO
		GLCall(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, _dbo, 0/*mipmap level*/));
		//-------------------------
		//Does the GPU support current FBO configuration?
		GLenum status;
		GLCall(status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return 1;
		default:
			ERROR("Frame buffer not good!");
			return 0;
		}

#else
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

		//vector<Color8> temp(width * height, Color8{});
		//memcpy(temp.data(), pixels, sizeof(Color8) * temp.size());

		//GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp.data()));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _surfaceToBeInitialized->pixels()));
		//GLCall(glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, NIGHT_OPENGL_TEXTURE_MIN_INTERNAL_RESOLUTION, NIGHT_OPENGL_TEXTURE_MIN_INTERNAL_RESOLUTION, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _id, 0));

		// depth buffer
		s32 dwidth = MAX(_surfaceToBeInitialized->width(), NIGHT_OPENGL_TEXTURE_DEPTH_BUFFER_MIN_SIZE);
		s32 dheight = MAX(_surfaceToBeInitialized->height(), NIGHT_OPENGL_TEXTURE_DEPTH_BUFFER_MIN_SIZE);
		GLCall(glGenTextures(1, &_dbo));
		GLCall(glBindTexture(GL_TEXTURE_2D, _dbo));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, dwidth, dheight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _dbo, 0));

		GLCall(glClear(GL_DEPTH_BUFFER_BIT));

		/*size({ _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height() });*/

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		TRACE("Texture created, id: ", _id, ", fbo: ", _fbo, ", dbo: ", _dbo);

		return 1;
#endif
	}

	void TextureOpenGL::clean()
	{
		ASSERT(_id != 0);
		ASSERT(_dbo != 0);
		ASSERT(_fbo != 0);

		TRACE("Destroying Texture, id: ", _id);

#if 0
		GLCall(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
		GLCall(glDeleteFramebuffersEXT(1, &_fbo));
#else
		GLCall(glDeleteFramebuffers(1, &_fbo));
#endif

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

	//void TextureOpenGL::bind_fbo() const
	//{
	//	ASSERT(_fbo != 0);
	//	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
	//	//GLCall(glViewport(0, 0, width(), height()));
	//}

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

			s32 dwidth = MAX(_pendingResize.x, NIGHT_OPENGL_TEXTURE_DEPTH_BUFFER_MIN_SIZE);
			s32 dheight = MAX(_pendingResize.y, NIGHT_OPENGL_TEXTURE_DEPTH_BUFFER_MIN_SIZE);
			GLCall(glBindTexture(GL_TEXTURE_2D, _dbo));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, dwidth, dheight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));

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