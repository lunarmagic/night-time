
#include "nightpch.h"
#include "NodeFrameBuffer.h"
#include "event/ApplicationEvent.h"
#include "texture/ITexture.h"

namespace night
{

	NodeFrameBuffer::NodeFrameBuffer(NodeFrameBufferParams const& params)
		: NodeRenderTarget( {
		.width = -1,
		.height = -1,
		.should_use_depth_peeling = params.should_use_depth_peeling,
		.should_use_depth_testing = params.should_use_depth_testing,
		.should_use_blending = params.should_use_blending
			})
	{
		ASSERT(_target != nullptr);

		{
			s32 w = _target->width();
			s32 h = _target->height();
			vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
			scale(vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f));
		}

		bind_event([&](WindowResizeEvent const& event)
			{
				s32 w = event.width();
				s32 h = event.height();
				vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
				scale(vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f));
			});

		clear_color = COLOR_ZERO;
	}

	void NodeFrameBuffer::on_render(RenderGraph& out_graph) const
	{
		utility::renderer().reset_state();
		utility::renderer().activate_textures(_target);
		// - (real)unique_id()/*TODO: unique id is not good for this*/ * 0.00001f

		//utility::renderer().draw_quad({ .position = vec3(0, 0, (real)-depth_from_root() * 0.01f), .size = vec2(scale()) });

		utility::renderer().draw_quad({ .position = vec3(0, 0, depth), .size = vec2(local_scale()) });
	}

#ifdef false
	void NodeFrameBuffer::on_pre_update_initialization()
	{
		__super::on_pre_update_initialization();

		ASSERT(_target != nullptr);

		{
			s32 w = _target->width();
			s32 h = _target->height();
			vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
			scale(vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f));
		}

		bind_event([&](WindowResizeEvent const& event)
		{
			s32 w = event.width();
			s32 h = event.height();
			vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
			scale(vec3(1.0f / ar.x, 1.0f / ar.y, 1.0f));
		});
	}
#endif

}