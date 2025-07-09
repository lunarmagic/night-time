
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

	//umap<string, NodeRenderTarget::RenderTargetCache> NodeRenderTarget::_renderTargetCache;

	NodeRenderTarget::NodeRenderTarget(NodeRenderTargetParams const& params)
	{
		init(params);
	}

#ifdef false
	void NodeRenderTarget::on_pre_update_initialization()
	{
		s32 w = _width != -1 ? _width : utility::renderer().width();
		s32 h = _height != -1 ? _height : utility::renderer().height();

		auto f = _renderTargetCache.find(name());
		if (f == _renderTargetCache.end())
		{
			Surface surface(SurfaceParams{ .width = w, .height = h, .fill_color = COLOR_ZERO });
			_target = utility::renderer().create_texture(RENDER_TARGET_PREFIX + name(), { .surface = &surface });
			ASSERT(_target != nullptr);
			_renderTargetCache.insert({ name(), { _target, true } });
		}
		else
		{
			ASSERT(!(*f).second.is_being_used); // DO NOT CREATE 2 RENDER TARGETS WITH THE SAME NAME!

			_target = (*f).second.target;
			(*f).second.is_being_used = true;
			ASSERT(_target != nullptr);
			if (_target->width() != w || _target->height() != h)
			{
				_target->resize({ w, h });
			}
		}

		_target->clear(COLOR_ZERO);

		Camera camera;
		camera.translation = FORWARD * 100.0f;
		camera.look_at = ORIGIN;
		camera.up = UP;
		camera.near_clip = RENDERER_DEFAULT_RENDER_TARGET_NEAR_CLIP;
		camera.far_clip = RENDERER_DEFAULT_RENDER_TARGET_FAR_CLIP;

		camera.type = ECameraType::Orthographic;
		camera.ortho_region = { .left = -1, .right = 1, .top = 1, .bottom = -1 };

		_target->camera(camera);

		textures(_target);

		_target->render_flush_priority((real)-depth_from_root());

		bind_event([&](WindowResizeEvent const& event)
		{
			if (_width == -1 || _height == -1)
			{
				ASSERT(_target != nullptr);
				_target->resize(ivec2(event.width(), event.height()));

				if (!should_automatically_render && !_isPendingManualRender)
				{
					_isPendingManualRender = true;
				}

				on_resize(ivec2(event.width(), event.height()));
			}
		});

		bind_event([&](NodeMovedEvent const& event)
		{
			ASSERT(target() != nullptr);
			_target->render_flush_priority((real)-depth_from_root()); // TODO: make sure this works.
		});
	}
#endif

	NodeRenderTarget::~NodeRenderTarget()
	{
#ifdef false
		constexpr s32 max_render_target_caches = 32;

		ASSERT(_target != nullptr);

		auto f = _renderTargetCache.find(name());
		ASSERT(f != _renderTargetCache.end()); // The Name of this node must not be changed

		if (_renderTargetCache.size() > max_render_target_caches)
		{
			ASSERT(utility::renderer().find_texture(RENDER_TARGET_PREFIX + name()) != nullptr);
			utility::renderer().destroy_texture(RENDER_TARGET_PREFIX + name());
			_renderTargetCache.erase(f);
		}
		else
		{
			(*f).second.is_being_used = false;
		}
#endif

		ASSERT(_target != nullptr);
		ASSERT(utility::renderer().find_texture(RENDER_TARGET_NAME) != nullptr);
		//utility::renderer().destroy_texture(RENDER_TARGET_NAME); // TODO: may want to save id
		utility::renderer().destroy_texture(_target);
	}

	void NodeRenderTarget::init(NodeRenderTargetParams const& params)
	{
		_width = params.width;
		_height = params.height;

		depth = params.depth;
		clear_color = params.clear_color;
		should_automatically_clear = params.should_automatically_clear;
		should_automatically_render = params.should_automatically_render;

		ivec2 resolution;
		if (_width == -1 || _height == -1)
		{
			auto crt = current_render_target();
			ASSERT(crt != nullptr);
			resolution.x = crt->width();
			resolution.y = crt->height();
		}
		else
		{
			resolution.x = _width;
			resolution.y = _height;
		}

		sref<Surface> surface(new Surface(SurfaceParams{ .width = resolution.x, .height = resolution.y, .fill_color = COLOR_ZERO }));
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
				handle_resize_event(event.width(), event.height());
			});

		bind_event([&](NodeMovedEvent const& event)
			{
				// TODO: get new size
				ASSERT(target() != nullptr);
				//target()->render_flush_priority((real)-depth_from_root()); // TODO: make sure this works.
				real rfp = (real)-depth_from_root();
				_target->render_flush_priority(rfp);
				on_render_flush_priority(rfp);

				auto crt = current_render_target();
				ASSERT(crt != nullptr);
				handle_resize_event(crt->width(), crt->height());

				//if (_width == -1 || _height == -1)
				//{
				//	auto crt = current_render_target();
				//	ASSERT(crt != nullptr);
				//	ASSERT(_target != nullptr);
				//	_target->resize(ivec2(crt->width(), crt->height()));
				//	_target->clear(clear_color);
				//	on_resize(ivec2(crt->width(), crt->height()));
				//	manually_render_this_frame();
				//}
			});
#if 0
		_width = params.width;
		_height = params.height;

		depth = params.depth;
		clear_color = params.clear_color;
		should_automatically_clear = params.should_automatically_clear;
		should_automatically_render = params.should_automatically_render;

		s32 w = _width != -1 ? _width : utility::renderer().width();
		s32 h = _height != -1 ? _height : utility::renderer().height();

		Surface surface(SurfaceParams{ .width = w, .height = h, .fill_color = COLOR_ZERO });
		_target = utility::renderer().create_texture(RENDER_TARGET_NAME, { .surface = &surface });
		ASSERT(_target != nullptr);

		Camera camera;
		camera.translation = FORWARD * 100.0f;
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

		target()->render_flush_priority((real)-depth_from_root());

		bind_event([&](WindowResizeEvent const& event)
			{
				if (_width == -1 || _height == -1)
				{
					ASSERT(_target != nullptr);
					_target->resize(ivec2(event.width(), event.height()));

					if (!should_automatically_render && !_isPendingManualRender)
					{
						_isPendingManualRender = true;
					}

					on_resize(ivec2(event.width(), event.height()));
				}
			});

		bind_event([&](NodeMovedEvent const& event)
			{
				ASSERT(target() != nullptr);
				target()->render_flush_priority((real)-depth_from_root()); // TODO: make sure this works.
			});
#endif
	}

	void NodeRenderTarget::handle_resize_event(u32 width, u32 height)
	{
		ASSERT(_target != nullptr);

		if (_width == -1 || _height == -1)
		{
			_target->resize(ivec2(width, height));
			on_resize(ivec2(width, height));
		}
		else
		{
			_target->resize(ivec2(_width, _height));
			on_resize(ivec2(_width, _height));
		}

		_target->clear(clear_color);
		manually_render_this_frame();
	}

	void NodeRenderTarget::resize(ivec2 const& new_size)
	{
		//ASSERT(target() != nullptr);
		_width = new_size.x;
		_height = new_size.y;

		WindowResizeEvent e(new_size.x, new_size.y);
		on_event(e); // dispatch resize in main thread for newly created children.

#if 0
		_target->resize(new_size);
		_target->clear(clear_color);
		on_resize(new_size);
		manually_render_this_frame();
		WindowResizeEvent e(new_size.x, new_size.y);
		pass_down_event(e);
#endif
	}

	void NodeRenderTarget::clear()
	{
		ASSERT(target() != nullptr);
		_target->clear(clear_color);
		on_clear(clear_color);
		manually_render_this_frame();
	}

	ivec2 NodeRenderTarget::size()
	{
		ASSERT(target() != nullptr);
		return ivec2(_target->width(), _target->height());
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
