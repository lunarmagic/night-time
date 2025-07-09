
#include "nightpch.h"
#include "VertexBufferLayoutOpenGL.h"
#include "log/log.h"
#include <GL/glew.h>

namespace night
{

	u32 VertexBufferElementOpenGL::sizeOfType(u32 type)
	{
		switch (type)
		{
		case GL_FLOAT:			return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}

		ASSERT(false); // "TYPE NOT FOUND"
		return 0;
	}

	template<>
	void VertexBufferLayoutOpenGL::push<real>(u32 count)
	{
		_elements.push_back({ GL_FLOAT, count, GL_FALSE });
		_stride += count * VertexBufferElementOpenGL::sizeOfType(GL_FLOAT);
	}

	template<>
	void VertexBufferLayoutOpenGL::push<u32>(u32 count)
	{
		_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		_stride += count * VertexBufferElementOpenGL::sizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	void VertexBufferLayoutOpenGL::push<u8>(u32 count)
	{
		_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		_stride += count * VertexBufferElementOpenGL::sizeOfType(GL_UNSIGNED_BYTE);
	}

}