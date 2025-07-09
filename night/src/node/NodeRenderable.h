#pragma once

#include "NodeSpatial.h"
//#include "renderer/RenderGraph.h"

namespace night
{
	enum class EVisibility : s32
	{
		Visible = 0,
		Invisible,
		Invisible_Tree
	};

	struct NodeRenderTarget;
	struct RenderGraph;
	struct DepthBuffer;

	struct NIGHT_API NodeRenderable : public NodeSpatial
	{
		template<typename Texture, typename... Textures>
		void textures(Texture texture_1, Textures... textures);
		void clear_textures() { _textures.clear(); }
		vector<TextureUniformData> const& textures() const { return _textures; }

		template<typename T>
		void uniform_buffer(T const& t);
		vector<u8> const& uniform_buffer() const { return _uniformBuffer; }

		static void render_node_tree(handle<INode> root, RenderGraph& out_graph);

		EVisibility visibility{ EVisibility::Visible };
		handle<IMaterial> material{ nullptr }; // TODO: may want system for multiple materials
		u8 should_skip_transformation_when_rendering{ false };

		// can return nullptr
		//handle<NodeRenderTarget> current_node_render_target() const;

	protected:

		virtual void on_render(RenderGraph& graph) const { return; }

		void manually_render_this_frame() const;

	private:

		vector<u8> _uniformBuffer{};
		vector<TextureUniformData> _textures{};
	};

	template<typename Texture, typename ...Textures>
	inline void NodeRenderable::textures(Texture texture_1, Textures ...textures)
	{
		_textures.clear();

		if constexpr (std::is_same_v<decltype(texture_1), DepthBuffer>)
		{
			TextureUniformData data;
			data.texture = texture_1.texture;
			data.sample_depth_buffer = true;
			_textures.push_back(data);
		}
		else
		{
			TextureUniformData data;
			data.texture = texture_1;
			data.sample_depth_buffer = false;
			_textures.push_back(data);
		}

		([&]
			{
				if constexpr (std::is_same_v<decltype(textures), DepthBuffer>)
				{
					TextureUniformData data;
					data.texture = textures.texture;
					data.sample_depth_buffer = true;
					_textures.push_back(data);
				}
				else
				{
					TextureUniformData data;
					data.texture = textures;
					data.sample_depth_buffer = false;
					_textures.push_back(data);
				}
			} (), ...);
	}

	template<typename T>
	inline void NodeRenderable::uniform_buffer(T const& t)
	{
		_uniformBuffer = vector<u8>(sizeof(T));
		memcpy(_uniformBuffer.data(), (void*)&t, sizeof(T));
	}

}