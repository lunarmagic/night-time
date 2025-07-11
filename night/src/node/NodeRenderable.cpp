
#include "nightpch.h"
#include "NodeRenderable.h"
#include "NodeRenderTarget.h"
#include "profiler/Profiler.h"
#include "renderer/RenderGraph.h"
#include "texture/ITexture.h"
#include "NodeFrameBuffer.h"
#include "NodeWindow.h"

namespace night
{
	//handle<NodeRenderTarget> NodeRenderable::current_node_render_target() const
	//{
	//	handle<INode> parent = this->parent();
	//	while (parent != nullptr)
	//	{
	//		NodeRenderTarget* dc = dynamic_cast<NodeRenderTarget*>(parent.ptr().lock().get());
	//		if (dc != nullptr)
	//		{
	//			return parent;
	//		}

	//		parent = parent->parent();
	//	}

	//	return nullptr;
	//}

	void NodeRenderable::manually_render_this_frame() const
	{
		//auto cnrt = current_node_render_target();
		auto cnrt = find_parent<NodeRenderTarget>();
		if (cnrt != nullptr)
		{
			cnrt->manually_render_this_frame();
		}
	}

	void NodeRenderable::render_node_tree(handle<INode> root, RenderGraph& out_graph)
	{
		//RenderGraph graph;

		// TODO: keep track of current render target iterator, material iterator, textures iterator
		// TODO: calculate relative transform and push it into transform uniform storage

		function<void(INode*, handle<const ITexture>, u8)> fn = [&](INode* node, handle<const ITexture> current_target, u8 should_skip_render)
		{
			ASSERT(node != nullptr);

			auto fn2 = [&]() -> u8
			{
				NodeRenderable* renderable = dynamic_cast<NodeRenderable*>(node);

				if (renderable != nullptr)
				{
					//if (renderable->name() == "Editor")
					//{
					//	ASSERT(false);
					//}
					out_graph.current_buffer(current_target, renderable->material, renderable->textures());

					if (!renderable->uniform_buffer().empty())
					{
						out_graph.append_uniform_buffer(renderable->uniform_buffer());
					}

					// TODO: calculate the global transform using the stack.
					if (!renderable->should_skip_transformation_when_rendering)
					{
						out_graph.append_transform(renderable->global_transform());
					}
					else
					{
						out_graph.append_transform(mat4(1));
					}

					// TODO: append transform buffer.
					if (renderable->visibility == EVisibility::Visible)
					{
						RenderGraph::Buffer* buffer = out_graph.current_buffer();
						ASSERT(!out_graph.transform_storage().empty());
						//ASSERT(out_graph.current_transform_index != -1);
						renderable->on_render(out_graph);
						out_graph.current_buffer(buffer);
					}
					else if (renderable->visibility == EVisibility::Invisible_Tree)
					{
						return true;
					}

					//// TODO: optimize this.
					//out_graph.current_buffer(current_target, renderable->material, renderable->textures());

					//if (!renderable->uniform_buffer().empty())
					//{
					//	out_graph.append_uniform_buffer(renderable->uniform_buffer());
					//}

					//// TODO: calculate the global transform using the stack.
					//out_graph.append_transform(renderable->global_transform());
				}

				return false;
			};

			NodeRenderTarget* target = dynamic_cast<NodeRenderTarget*>(node);
			u8 ssr = false;
			u8 ret = false; // return if node is invisible

			if (target != nullptr)
			{
				// if we are a render target, render to current_target,
				// then update current_target, handle manual rendering,
				if (target->should_automatically_render || target->_isPendingManualRender)
				{
					if (target->should_automatically_clear)
					{
						ASSERT(target->target() != nullptr);
						//ASSERT(target->name() != "Canvas");
						//target->target()->clear(target->clear_color);
						target->clear();
					}
				}
				else
				{
					ssr = true;
				}

				// always render framebuffers, only render rendertargets if their
				// crt is not skipping rendering
				if (/*node->is_of_type<NodeFrameBuffer>() ||*/ !should_skip_render)
				{
					ret = fn2(); // TODO: may cause input-lag doing this before the children.
				}

				current_target = target->target();
				target->_isPendingManualRender = false;
			}
			else if (!should_skip_render)
			{
				// else, render the children with the pending_manual_render of the current_target
				// until we find the next target.
				ret = fn2();
			}

			if (ret)
			{
				return;
			}

			for (const auto& i : node->children())
			{
				fn(i.get(), current_target, ssr);
			}
		};

		fn(root.ptr().lock().get(), utility::renderer().default_render_target(), false);

		//utility::renderer().flush_render_graph(out_graph); // TODO: may want to return graph, call this in application
	}
}