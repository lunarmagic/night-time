
#include "nightpch.h"
#include "ShaderStorageBufferOpenGL.h"
#include <GL/glew.h>
#include "GLCall.h"
#include "log/log.h"

namespace night
{

	void ShaderStorageBufferOpenGL::init(u32 binding)
	{
		GLCall(glGenBuffers(1, &_rendererID));
		_binding = binding;
	}

	void ShaderStorageBufferOpenGL::clean()
	{
		GLCall(glDeleteBuffers(1, &_rendererID));
		_rendererID = 0;
		_size = 0;
	}

	void ShaderStorageBufferOpenGL::bind() const
	{
		GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, _rendererID));
	}

	void ShaderStorageBufferOpenGL::unbind() const
	{
		GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	}

	void ShaderStorageBufferOpenGL::reserve(size_t size)
	{
		bind();
		vector<u8> d(size, 0); // TODO: remove this.
		GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, d.data(), GL_STATIC_DRAW));
		GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _binding, _rendererID));
	}

	void ShaderStorageBufferOpenGL::data(const void* data, size_t size)
	{
		bind();
		GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW));
		GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _binding, _rendererID));

		_size = size;
	}

	void ShaderStorageBufferOpenGL::data(void* out_data)
	{
		bind();
		ASSERT(_size != 0);
		GLCall(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, _size, out_data));
	}
}