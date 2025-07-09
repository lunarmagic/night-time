#pragma once

//#include "stl/stl.h"

namespace night
{

	struct VertexBufferElementOpenGL
	{
		u32 type;
		u32 count;
		u8 normalized;

		static u32 sizeOfType(u32 type);
	};

	struct VertexBufferLayoutOpenGL
	{
		VertexBufferLayoutOpenGL() : _stride(0) {}

		void clean()
		{
			_elements.clear();
			_stride = 0;
		}

		template<typename T>
		void push(u32 count);

		inline const vector<VertexBufferElementOpenGL>& elements() const { return _elements; }

		inline u32 stride() const { return _stride; }

	private:

		vector<VertexBufferElementOpenGL> _elements;
		u32 _stride;
	};

}

