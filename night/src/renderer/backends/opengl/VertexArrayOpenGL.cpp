
#include "nightpch.h"
#include "VertexArrayOpenGL.h"
#include "GLCall.h"
#include "GL/glew.h"
//#include "VertexBufferLayout.h"
//#include "BatchRendererGL.h"
//#include "geometry/Quad.h"

namespace night
{
	void VertexArrayOpenGL::init()
	{
		GLCall(glGenVertexArrays(1, &_rendererID));
	}

	void VertexArrayOpenGL::clean()
	{
		GLCall(glDeleteVertexArrays(1, &_rendererID));
		_rendererID = 0;
	}

	void VertexArrayOpenGL::bind() const
	{
		GLCall(glBindVertexArray(_rendererID));
	}

	void VertexArrayOpenGL::unbind() const
	{
		GLCall(glBindVertexArray(0));
	}

	void VertexArrayOpenGL::addBufferImpl(const VertexBufferLayoutOpenGL& layout)
	{
		const auto elements = layout.elements();
		uint64_t offset = 0u;
		for (auto i = 0u; i < elements.size(); i++)
		{
			const auto& element = elements[i];
			GLCall(glEnableVertexAttribArray(i));
			GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.stride(), (const void*)offset));
			offset += element.count * VertexBufferElementOpenGL::sizeOfType(element.type);
		}
	}

}

