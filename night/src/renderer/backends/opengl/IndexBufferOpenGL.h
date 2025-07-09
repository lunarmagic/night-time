#pragma once

//#include "stl/stl.h"

namespace night
{

	struct IndexBufferOpenGL
	{
		IndexBufferOpenGL() = default;

		void init(const u32* data, u32 count);
		void clean();

		void bind() const;
		void unbind() const;

		inline u32 count() const { return _count; }

	private:

		u32 _rendererID{ 0 };
		u32 _count{ 0 };
	};

}

