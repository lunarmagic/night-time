
#include "nightpch.h"
#include "VertexBufferOpenGL.h"
#include <GL/glew.h>
#include "GLCall.h"

namespace night
{

	void __vbo_init(u32& renderer_id, const void* data, u32 size)
	{
		GLCall(glGenBuffers(1, &renderer_id));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id));
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	}

	void __vbo_init(u32& renderer_id, u32 size)
	{
		GLCall(glGenBuffers(1, &renderer_id));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id));
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
	}

	void __vbo_clean(u32& renderer_id)
	{
		GLCall(glDeleteBuffers(1, &renderer_id));
	}

	void __vbo_data(u32& renderer_id, const void* data, u32 size)
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id));
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	}

	void* __vbo_begin(u32& renderer_id)
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id));
		void* buffer;
		GLCall(buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		return buffer;
	}

	void __vbo_end()
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	void __vbo_bind(u32 const& renderer_id)
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id));
	}

	void __vbo_unbind()
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

}