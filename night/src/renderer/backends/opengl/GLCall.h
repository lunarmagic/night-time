#pragma once

#include <GL/glew.h>
//#include <iostream>

#define _GL_CALL // TODO: put in premake file

#ifdef _GL_CALL
#define GLASSERT(x) if(!(x)) __debugbreak()
#define GLCall(x) GLClearError();\
    x;\
    GLASSERT(GLLogCall(#x, __FILE__, __LINE__))

inline void GLClearError()
{
	GLenum e = glGetError();
	while (e != GL_NO_ERROR)
	{
		e = glGetError();
	}
}

inline bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << std::hex << error << " hex), " << \
			function << ", " << file << ": " << std::dec << line << std::endl;
		return false;
	}
	return true;
};
#else
#define GLASSERT(x)
#define GLCall(x) x;
#endif
