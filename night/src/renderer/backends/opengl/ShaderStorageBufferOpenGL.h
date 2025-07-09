#pragma once

//#include "types/types.h"
//#include "stl/stl.h"
#include "log/log.h"

namespace night
{

	struct ShaderStorageBufferOpenGL
	{
		void init(u32 binding = 0);
		void clean();

		void bind() const;
		void unbind() const;

		void reserve(size_t size);

		void data(const void* data, size_t size);

		void data(void* out_data);

		u32 id() const { return _rendererID; }

		size_t const& size() const { return _size; }

	private:

		u32 _rendererID{ 0 };
		u32 _binding{ 0 };
		size_t _size{ 0 };
	};

}