#pragma once

#include "core.h"
//#include "Surface.h"
#include "math/Math.h"
//#include "raycast/Raycast.h"
#include "camera/Camera.h"
#include "log/log.h"
#include "ref/ref.h"
#include "handle/handle.h"
#include "color/Color.h"
#include "resource_manager/IResource.h"
//#include "geometry/Quad.h"
#include "aabb/AABB.h"
#include "TextureUniformData.h"

namespace night
{
	struct WindowSDL;
	struct Surface;

	template<typename T>
	struct Ray3D;

	struct DepthBuffer;

	enum class ETextureFiltering
	{
		Nearest = 0,
		Linear
	};

	struct TextureParams
	{
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

		Ray3D<real> mouse_pick(vec2 const& mouse_position) const; // TODO: these may not need to be virtual
		vec4 project(vec3 const& point) const;
		vec3 unproject(vec3 const& point) const;
		vec3 unproject(vec2 const& point) const;

		virtual void on_resize(ivec2 const& new_size) = 0;
		virtual void on_clear(Color const& clear_color) = 0;

		void resize(ivec2 const& new_size);
		void clear(Color const& clear_color);

		void render_flush_priority(real priority)
		{
			_renderFlushPriority = priority;
		}

		real render_flush_priority() const { return _renderFlushPriority; }

		vec2 global_to_local(vec2 const& global) const;
		vec2 local_to_global(vec2 const& local) const;
		ivec2 local_to_internal(vec2 const& local) const;
		ivec2 global_to_internal(vec2 const& global) const;
		vec2 internal_to_local(ivec2 const& internal) const;
		vec2 internal_to_global(ivec2 const& internal) const;

		b8 should_use_blending{ true };
		b8 should_use_depth_testing{ true };
		b8 should_use_depth_peeling{ true };

		//static void update_textures();

		DepthBuffer depth_buffer() const;

		vec2 aspect_ratio() const;

		AABB<> area() const;

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

	private:

		s32 _width{ 0 };
		s32 _height{ 0 };
		mat4 _mvp{ mat4(1) };
		Camera _camera; // TODO: figure out if we need this.
		real _renderFlushPriority{ 0.0f };
	};

	// TODO: may want to add ref
	struct CompareITextureSHandleByRenderFlushPriority
	{
		b8 operator()(shandle<const ITexture> const& a, shandle<const ITexture> const& b) const
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
		handle<const ITexture> texture; // TODO: change to handle

		//operator TextureUniformData() const
		//{
		//	TextureUniformData data;
		//	data.texture = this->texture;
		//	data.sample_depth_buffer = true;
		//
		//	return data;
		//}
	};

}