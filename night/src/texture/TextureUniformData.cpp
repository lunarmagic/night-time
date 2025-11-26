
#include "nightpch.h"
#include "TextureUniformData.h"
#include "ITexture.h"

namespace night
{
	TextureUniformData::TextureUniformData(DepthBuffer const& depth_buffer)
		: texture(depth_buffer.texture.ptr().lock())
		, sample_depth_buffer(true)
	{
	}
}