
#include "nightpch.h"
#include "NodeRenderTarget.h"
#include "utility.h"
#include "event/ApplicationEvent.h"
#include "event/NodeEvent.h"
#include "raycast/raycast.h"
#include "texture/Surface.h"
#include "camera/Camera.h"
#include "texture/ITexture.h"
#include "thread/Thread.h"

#define RENDER_TARGET_PREFIX "Render Target "
#define RENDER_TARGET_NAME RENDER_TARGET_PREFIX + name() + to_string(unique_id())

namespace night
{

	NodeRenderTarget::NodeRenderTarget(NodeRenderTargetParams const& params)
	{
		init(params);
	}

	NodeRenderTarget::~NodeRenderTarget()
	{
		ASSERT(_target != nullptr);
		ASSERT(utility::renderer().find_texture(RENDER_TARGET_NAME) != nullptr);
		utility::renderer().destroy_texture(_target);
	}

	void NodeRenderTarget::init(NodeRenderTargetParams const& params)
	{
		should_inherit_parent_resolution = params.should_inherit_parent_resolution;
		_pendingWidth = params.width;
		_pendingHeight = params.height;

		depth = params.depth;
		clear_color = params.clear_color;
		should_automatically_clear = params.should_automatically_clear;
		should_automatically_render = params.should_automatically_render;

		ivec2 resolution;
		if (should_inherit_parent_resolution)
		{
			auto crt = current_render_target();
			ASSERT(crt != nullptr);
			resolution.x = crt->width();
			resolution.y = crt->height();
		}
		else
		{
			resolution.x = _pendingWidth;
			resolution.y = _pendingHeight;
		}

		sref<Surface> surface(new Surface(SurfaceParams{ .width = resolution.x, .height = resolution.y, .fill_color = clear_color }));
		_target = utility::renderer().create_texture(RENDER_TARGET_NAME, { .surface = surface, .filtering = params.filtering });
		ASSERT(_target != nullptr);

		Camera camera;
		camera.translation = FORWARD * (RENDERER_DEFAULT_RENDER_TARGET_FAR_CLIP / 2.0f);
		camera.look_at = ORIGIN;
		camera.up = UP;
		camera.near_clip = RENDERER_DEFAULT_RENDER_TARGET_NEAR_CLIP;
		camera.far_clip = RENDERER_DEFAULT_RENDER_TARGET_FAR_CLIP;

		camera.type = ECameraType::Orthographic;
		camera.ortho_region = { .left = -1, .right = 1, .top = 1, .bottom = -1 };

		_target->camera(camera);

		_target->should_use_blending = params.should_use_blending;
		_target->should_use_depth_peeling = params.should_use_depth_peeling;
		_target->should_use_depth_testing = params.should_use_depth_testing;

		textures(_target);

		real rfp = (real)-depth_from_root();
		_target->render_flush_priority(rfp);
		on_render_flush_priority(rfp);


		bind_event([&](WindowResizeEvent const& event)
			{
				handle_resize(event.width(), event.height());
			});

		bind_event([&](NodeMovedEvent const& event)
			{
				ASSERT(target() != nullptr);
				real rfp = (real)-depth_from_root();
				_target->render_flush_priority(rfp);
				on_render_flush_priority(rfp);

				auto crt = current_render_target();
				ASSERT(crt != nullptr);
				handle_resize(crt->width(), crt->height());
			});
	}

	void NodeRenderTarget::handle_resize(u32 width, u32 height)
	{
		ASSERT(_target != nullptr);

		if(should_inherit_parent_resolution)
		{
			_pendingWidth = width; // we use _pendingWidth and height when getting resolution
			_pendingHeight = height;
			_target->resize(ivec2(width, height));
			on_resize(ivec2(width, height));
		}
		else
		{
			if (_target->width() != _pendingWidth || _target->height() != _pendingHeight)
			{
				_target->resize(ivec2(_pendingWidth, _pendingHeight));
				on_resize(ivec2(_pendingWidth, _pendingHeight));
			}
		}

		_target->clear(clear_color);
		manually_render_this_frame();
	}

	void NodeRenderTarget::resize(ivec2 const& new_size)
	{
		_pendingWidth = new_size.x;
		_pendingHeight = new_size.y;

		WindowResizeEvent e(new_size.x, new_size.y);
		on_event(e); // dispatch resize in main thread for newly created children.
	}

	void NodeRenderTarget::clear()
	{
		ASSERT(target() != nullptr);
		_target->clear(clear_color);
		on_clear(clear_color);
		manually_render_this_frame();
	}

	void NodeRenderTarget::camera(Camera camera_to_be_set)
	{
		ASSERT(target() != nullptr);
		on_camera(camera_to_be_set);
		_target->camera(camera_to_be_set);
		manually_render_this_frame();
	}

	Camera const& NodeRenderTarget::camera() const
	{
		ASSERT(target() != nullptr);
		return _target->camera();
	}

	Ray NodeRenderTarget::mouse_pick(vec2 const& mouse_position) const
	{
		ASSERT(target() != nullptr);
		return _target->mouse_pick(mouse_position);
	}

	vec4 NodeRenderTarget::project_to_screen(vec3 const& point) const
	{
		ASSERT(target() != nullptr);
		return _target->project_to_screen(point);
	}

	real NodeRenderTarget::render_flush_priority() const
	{
		ASSERT(target() != nullptr);
		return _target->render_flush_priority();
	}

	ivec2 NodeRenderTarget::global_to_internal(vec2 const& global) const
	{
		ASSERT(target() != nullptr);
		return _target->global_to_internal(global);
	}

	vec2 NodeRenderTarget::internal_to_global(ivec2 const& internal) const
	{
		ASSERT(target() != nullptr);
		return _target->internal_to_global(internal);
	}

	void NodeRenderTarget::should_use_blending(u8 x)
	{
		ASSERT(target() != nullptr);
		_target->should_use_blending = x;
	}

	void NodeRenderTarget::should_use_depth_testing(u8 x)
	{
		ASSERT(target() != nullptr);
		_target->should_use_depth_testing = x;
	}

	void NodeRenderTarget::should_use_depth_peeling(u8 x)
	{
		ASSERT(target() != nullptr);
		_target->should_use_depth_peeling = x;
	}

}
