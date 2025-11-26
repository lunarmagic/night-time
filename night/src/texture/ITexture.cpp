
#include "nightpch.h"
#include "ITexture.h"
//#include "renderer/IRenderer.h"
#include "raycast/Raycast.h"
#include "color/Color.h"
#include "Surface.h"
#include "utility.h"
#include "application/Application.h"
//#include <glm/gtc/matrix_transform.hpp>
//#include "geometry/Quad.h"

namespace night
{
	//set<handle<ITexture>> ITexture::_toBeInitialized;
	//set<handle<ITexture>> ITexture::_toBeResized;
	//set<handle<ITexture>> ITexture::_toBeCleared;

	ITexture::ITexture(TextureParams const& params, string const& id)
		: IResource(id, params.surface == nullptr ? params.path : "loaded from surface")
	{
		ASSERT(!(!params.path.empty() && params.surface != nullptr));
		if (params.path.empty() && params.surface != nullptr)
		{
			_surfaceToBeInitialized = params.surface;
		}
		else
		{
			_surfaceToBeInitialized = sref<Surface>(new Surface(SurfaceParams{ .path = params.path }));
		}

		ASSERT(_surfaceToBeInitialized != nullptr);

		_filtering = params.filtering;

		size({ _surfaceToBeInitialized->width(), _surfaceToBeInitialized->height() });
	}

	void ITexture::__tempInitHandle()
	{
		//static mutex m;
		//m.lock();
		//_toBeInitialized.insert(handle_from_this());
		//m.unlock();

		Application::get().queue_for_main_thread([self = (handle<ITexture>)handle_from_this()]()
			{
				ASSERT(self != nullptr);
				self->init();
			}
		);
	}

	void ITexture::camera(Camera const& camera)
	{
		_camera = camera;
		update_mvp();
	}

	void ITexture::update_mvp()
	{
		mat4 view = math::look_at(_camera.translation, _camera.look_at, _camera.up);

		mat4 projection;

		if (_camera.type == ECameraType::Perspective)
		{
			projection = math::perspective(RADIANS(_camera.fov), (real)width() / (real)height(), _camera.near_clip, _camera.far_clip);
		}
		else if (_camera.type == ECameraType::Orthographic)
		{
			projection = math::ortho(_camera.ortho_region.left, _camera.ortho_region.right, _camera.ortho_region.bottom, _camera.ortho_region.top, _camera.near_clip, _camera.far_clip);

			s32 w = width();
			s32 h = height();
			vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
			mat4 scale = math::scale(vec3(ar.x, ar.y, 1.0f)); // we need to manualy scale for ortho

			projection *= scale;
		}

		_mvp = projection * view;
	}

	Ray3D<real> ITexture::mouse_pick(vec2 const& mouse_position) const
	{
		vec3 eye = math::unproject(vec3(mouse_position.x, mouse_position.y, 0.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		vec3 forward = math::unproject(vec3(mouse_position.x, mouse_position.y, 1.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));

		vec3 direction = math::normalize(forward - eye);
		return { .origin = eye, .direction = direction };
	}

	vec4 ITexture::project(vec3 const& point) const
	{
		return vec4(math::project(point, mat4(1), _mvp, vec4(-1, -1, 2, 2)), 1);
	}

	vec3 ITexture::unproject(vec3 const& point) const
	{
		vec3 result = math::unproject(vec3(point.x, point.y, point.z), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		return result;
	}

	vec3 ITexture::unproject(vec2 const& point) const
	{
		vec3 result = math::unproject(vec3(point.x, point.y, 0.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		return result;
	}

	void ITexture::resize(ivec2 const& new_size)
	{
		size(new_size);
		on_resize(new_size);
	}

	void ITexture::clear(Color const& clear_color)
	{
		on_clear(clear_color);
	}

	vec2 ITexture::global_to_local(vec2 const& global) const
	{
		//return project(vec3(global, 0));
		vec2 ar = aspect_ratio();
		return global * ar;
	}

	vec2 ITexture::local_to_global(vec2 const& local) const
	{
		//return (vec2)mouse_pick(local).origin;
		vec2 ar = aspect_ratio();
		return local / ar;
	}

	ivec2 ITexture::local_to_internal(vec2 const& local) const
	{
		ivec2 result;
		result.x = (s32)(((local.x + 1.0f) / 2.0f) * width());
		result.y = (s32)(((local.y + 1.0f) / 2.0f) * height());
		return result;
	}

	ivec2 ITexture::global_to_internal(vec2 const& global) const
	{
		vec2 local = global_to_local(global);
		ivec2 result;
		result.x = (s32)(((local.x + 1.0f) / 2.0f) * width());
		result.y = (s32)(((local.y + 1.0f) / 2.0f) * height());
		return result;
	}

	vec2 ITexture::internal_to_local(ivec2 const& internal) const
	{
		vec2 result;
		result.x = ((real)internal.x / width()) * 2.0f - 1.0f;
		result.y = ((real)internal.y / height()) * 2.0f - 1.0f;
		return result;
	}

	vec2 ITexture::internal_to_global(ivec2 const& internal) const
	{
		vec2 local;
		local.x = ((real)internal.x / width()) * 2.0f - 1.0f;
		local.y = ((real)internal.y / height()) * 2.0f - 1.0f;
		return local_to_global(local);
	}

	DepthBuffer ITexture::depth_buffer() const
	{
		return DepthBuffer{ .texture = handle_from_this_const() };
	}

	vec2 ITexture::aspect_ratio() const
	{
		s32 w = width();
		s32 h = height();
		vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
		return ar;
	}

	AABB<> ITexture::area() const
	{
		s32 w = width();
		s32 h = height();
		vec2 ar = { (h < w ? (real)w / (real)h : 1.0f), (w < h ? (real)h / (real)w : 1.0f) };

		AABB<> result;
		result.left = -ar.x;
		result.right = ar.x;
		result.top = ar.y;
		result.bottom = -ar.y;

		return result;
	}

}
