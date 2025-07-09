#pragma once

#include "VertexBufferOpenGL.h"
#include "VertexBufferLayoutOpenGL.h"
//#include "types/types.h"
//#include "GLCall.h"
//#include <GL/glew.h>

namespace night
{

	//struct VertexBufferLayout;

	struct VertexArrayOpenGL
	{
		VertexArrayOpenGL() = default;

		void init();
		void clean();

		void bind() const;
		void unbind() const;

		template<typename T>
		void addBuffer(const VertexBufferOpenGL<T>& vb, const VertexBufferLayoutOpenGL& layout)
		{
			bind();
			vb.bind();
			addBufferImpl(layout);
			//const auto elements = layout.elements();
			//uint64_t offset = 0u;
			//for (auto i = 0u; i < elements.size(); i++)
			//{
			//	const auto& element = elements[i];
			//	GLCall(glEnableVertexAttribArray(i));
			//	GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.stride(), (const void*)offset));
			//	offset += element.count * VertexBufferElementOpenGL::sizeOfType(element.type);
			//}
		}

	private:

		void addBufferImpl(const VertexBufferLayoutOpenGL& layout);

		u32 _rendererID{ 0 };
	};

}

