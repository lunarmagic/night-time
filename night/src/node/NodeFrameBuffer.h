#pragma once

#include "NodeRenderTarget.h"

namespace night
{

	struct NodeFrameBufferParams
	{
		u8 should_use_depth_peeling{ true };
		u8 should_use_depth_testing{ true };
		u8 should_use_blending{ true };
	};

	// like NodeRenderTarget, but it clears automatically and resizes with window.
	struct NIGHT_API NodeFrameBuffer : public NodeRenderTarget
	{
		NodeFrameBuffer(NodeFrameBufferParams const& params = {});

		virtual void on_render(RenderGraph& out_graph) const  override;
		// TODO: add resolution mod, multiply when resized

	protected:

		//virtual void on_pre_update_initialization() override;
	};

}