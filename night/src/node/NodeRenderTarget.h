#pragma once

#include "NodeRenderable.h"
#include "texture/ITexture.h"

namespace night
{
	struct ITexture;
	struct Color;
	struct Ray;

	struct NodeRenderTargetParams
	{
		s32 width{ -1 };
		s32 height{ -1 };
		real depth{ 0.0f };

		Color clear_color{ COLOR_ZERO };

		u8 should_use_depth_peeling{ true };
		u8 should_use_depth_testing{ true };
		u8 should_use_blending{ true };
		u8 should_automatically_clear{ true };
		u8 should_automatically_render{ true };

		ETextureFiltering filtering{ETextureFiltering::Nearest};
	};

	struct NIGHT_API NodeRenderTarget : public NodeRenderable
	{
		NodeRenderTarget(NodeRenderTargetParams const& params);
		~NodeRenderTarget();

		// always call this when resizing render target, never call texture resize.
		void resize(ivec2 const& new_size);
		//void clear(Color const& clear_color);
		virtual void clear();

		ivec2 size();
		void camera(Camera camera);
		Camera const& camera() const;
		Ray mouse_pick(vec2 const& mouse_position) const; // TODO: these may not need to be virtual
		vec4 project_to_screen(vec3 const& point) const;
		real render_flush_priority() const;
		ivec2 global_to_internal(vec2 const& global) const;
		vec2 internal_to_global(ivec2 const& internal) const;

		void should_use_blending(u8 x);
		void should_use_depth_testing(u8 x);
		void should_use_depth_peeling(u8 x);

		Color clear_color{ COLOR_ZERO };
		u8 should_automatically_clear{ true };
		u8 should_automatically_render{ true };

		real depth{ 0.0f };

		void manually_render_this_frame() { _isPendingManualRender = true; }
		u8 const& is_pending_manual_render() const { return _isPendingManualRender; }
		//virtual void clear();

		handle<const ITexture> target() const { return _target; }

		s32 const& width() const { return _width; }
		s32 const& height() const { return _height; }

	protected:

		NodeRenderTarget() = default;
		void init(NodeRenderTargetParams const& params);

		virtual void on_resize(ivec2 const& new_size) {};
		virtual void on_clear(Color const& clear_color) {};
		virtual void on_camera(Camera& camera_to_be_set) {};
		virtual void on_render_flush_priority(real priority) {};
		//virtual void on_pre_update_initialization() override;

		void handle_resize_event(u32 width, u32 height);

		//handle<ITexture> const& target() const { return _target; }

		// do not destroy the target from subclass
		handle<ITexture> _target{ nullptr };
		
		s32 _width{ -1 };
		s32 _height{ -1 };

	private:


		u8 _isPendingManualRender{ false };

		//struct RenderTargetCache
		//{
		//	handle<ITexture> target;
		//	u8 is_being_used{ false };
		//};

		//static umap<string, RenderTargetCache> _renderTargetCache;

		friend struct NodeRenderable;
	};

}