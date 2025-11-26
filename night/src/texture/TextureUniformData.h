#pragma once

#include "core.h"
#include "handle/handle.h"
#include "stl/stl.h"

namespace night
{
	struct ITexture;
	struct DepthBuffer;

	struct NIGHT_API TextureUniformData
	{
		TextureUniformData() = default;

		TextureUniformData(DepthBuffer const& depth_buffer);

		TextureUniformData(shandle<const ITexture> const& texture)
			: texture(texture)
			, sample_depth_buffer(false)
		{}

		TextureUniformData(shandle<ITexture> const& texture)
			: texture(texture)
			, sample_depth_buffer(false)
		{
		}

		TextureUniformData(handle<ITexture> const& texture)
			: texture(texture.ptr().lock())
			, sample_depth_buffer(false)
		{
		}

		TextureUniformData(handle<const ITexture> const& texture)
			: texture(texture.ptr().lock())
			, sample_depth_buffer(false)
		{
		}

		shandle<const ITexture> texture{ nullptr };
		b8 sample_depth_buffer{ false };

		b8 operator<(TextureUniformData const& rhs) const
		{
			return (texture.get() < rhs.texture.get() || sample_depth_buffer < rhs.sample_depth_buffer);
		}

		b8 operator==(TextureUniformData const& other) const
		{
			return (texture == other.texture && sample_depth_buffer == other.sample_depth_buffer);
		}
	};
}