#pragma once

#include "core.h"
//#include "Surface.h"
#include "math/math.h"
//#include "raycast/raycast.h"
#include "camera/Camera.h"
#include "log/log.h"
#include "ref/ref.h"
#include "handle/handle.h"
#include "color/Color.h"
#include "resource_manager/IResource.h"
//#include "geometry/Quad.h"
#include "aabb/AABB.h"

namespace night
{
	struct WindowSDL;
	struct Surface;
	struct Ray;
	//struct Color;

	struct DepthBuffer;

	enum class ETextureFiltering
	{
		Nearest = 0,
		Linear
	};

	struct TextureParams
	{
		//Surface* surface{ nullptr };
		sref<Surface> surface{ nullptr };
		string path{ "" };
		ETextureFiltering filtering{ ETextureFiltering::Nearest };
	};

	// TODO: sort frame buffers by depth, so we render them in correct order if a frame buffer contains another frame buffer,
	// default framebuffer will have depth -1, NodeFrameBuffers will have depths equal to there depth in the node tree
	struct NIGHT_API ITexture : public IResource
	{
		ITexture(TextureParams const& params, string const& id);

		virtual void __tempInitHandle() override;

		virtual s32 init() = 0;
		virtual void clean() = 0;
		s32 width() const { return _width; };
		s32 height() const { return _height; };

		// when dealing with cameras, it makes most sense to only have 1 camera per render target.
		void camera(Camera const& camera);
		Camera const& camera() const { return _camera; }

		mat4 const& mvp() const { return _mvp; }

		Ray mouse_pick(vec2 const& mouse_position) const; // TODO: these may not need to be virtual
		vec4 project_to_screen(vec3 const& point) const;
		vec3 unproject_from_screen(vec3 const& point) const;
		vec3 unproject_from_screen(vec2 const& point) const;

		virtual void on_resize() = 0;
		virtual void on_clear() = 0;

		void resize(ivec2 const& new_size);
		void clear(Color const& clear_color);

		void render_flush_priority(real priority)
		{
			_renderFlushPriority = priority;
		}

		real render_flush_priority() const { return _renderFlushPriority; }

		ivec2 global_to_internal(vec2 const& global) const;
		vec2 internal_to_global(ivec2 const& internal) const;

		u8 should_use_blending{ true };
		u8 should_use_depth_testing{ true };
		u8 should_use_depth_peeling{ true };

		static void update_textures();

		DepthBuffer depth_buffer() const;

		//Quad area() const;
		AABB area() const;

	protected:

		void size(ivec2 size)
		{
			_width = size.x;
			_height = size.y;
			update_mvp();
		}

		// only to be called when updating texture width / height
		void update_mvp();

		sref<Surface> _surfaceToBeInitialized;
		ETextureFiltering _filtering;

		ivec2 _pendingResize{ ivec2(-1, -1) };
		Color _pendingClear{ Color(-1, -1, -1, -1) };

	private:

		s32 _width{ 0 };
		s32 _height{ 0 };
		mat4 _mvp{ mat4(1) };
		Camera _camera; // TODO: figure out if we need this.
		real _renderFlushPriority{ 0.0f };

		static set<handle<ITexture>> _toBeInitialized;
		static set<handle<ITexture>> _toBeResized;
		static set<handle<ITexture>> _toBeCleared;
	};

	// TODO: may want to add ref
	struct CompareITextureSHandleByRenderFlushPriority
	{
		u8 operator()(shandle<const ITexture> const& a, shandle<const ITexture> const& b) const
		{
			ASSERT(a != nullptr && b != nullptr);
			if (a->render_flush_priority() == b->render_flush_priority())
			{
				return (a < b);
			}

			return (a->render_flush_priority() < b->render_flush_priority());
		}
	};

	struct DepthBuffer
	{
		handle<const ITexture> texture;
	};

}