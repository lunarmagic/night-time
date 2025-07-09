
#include "nightpch.h"
#include "ITexture.h"
//#include "renderer/IRenderer.h"
#include "raycast/raycast.h"
#include "color/Color.h"
#include "Surface.h"
#include "utility.h"
#include <glm/gtc/matrix_transform.hpp>
//#include "geometry/Quad.h"

namespace night
{
	set<handle<ITexture>> ITexture::_toBeInitialized;
	set<handle<ITexture>> ITexture::_toBeResized;
	set<handle<ITexture>> ITexture::_toBeCleared;

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
		static mutex m;
		m.lock();
		_toBeInitialized.insert(handle_from_this());
		m.unlock();
	}

	void ITexture::camera(Camera const& camera)
	{
		_camera = camera;
		update_mvp();
	}

	void ITexture::update_mvp()
	{
		glm::mat4 view = glm::lookAt(_camera.translation, _camera.look_at, _camera.up);

		glm::mat4 projection;

		if (_camera.type == ECameraType::Perspective)
		{
			projection = glm::perspective(glm::radians(_camera.fov), (real)width() / (real)height(), _camera.near_clip, _camera.far_clip);
		}
		else if (_camera.type == ECameraType::Orthographic)
		{
			projection = glm::ortho(_camera.ortho_region.left, _camera.ortho_region.right, _camera.ortho_region.bottom, _camera.ortho_region.top, _camera.near_clip, _camera.far_clip);

			s32 w = width();
			s32 h = height();
			vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
			mat4 scale = glm::scale(mat4(1), vec3(ar.x, ar.y, 1.0f)); // we need to manualy scale for ortho

			projection *= scale;
		}

		_mvp = projection * view;
	}

	Ray ITexture::mouse_pick(vec2 const& mouse_position) const
	{
		vec3 eye = glm::unProject(vec3(mouse_position.x, mouse_position.y, 0.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		vec3 forward = glm::unProject(vec3(mouse_position.x, mouse_position.y, 1.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));

		vec3 direction = normalize(forward - eye);
		return { .origin = eye, .direction = direction };
	}

	vec4 ITexture::project_to_screen(vec3 const& point) const
	{
		return vec4(glm::project(point, mat4(1), _mvp, vec4(-1, -1, 2, 2)), 1);
	}

	vec3 ITexture::unproject_from_screen(vec3 const& point) const
	{
		vec3 result = glm::unProject(vec3(point.x, point.y, point.z), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		return result;
	}

	vec3 ITexture::unproject_from_screen(vec2 const& point) const
	{
		vec3 result = glm::unProject(vec3(point.x, point.y, 0.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		return result;
	}

	void ITexture::resize(ivec2 const& new_size)
	{
		_pendingResize = new_size;
		size(_pendingResize);
		//utility::renderer().resize_texture(handle_from_this());
		static mutex m;
		m.lock();
		_toBeResized.insert(handle_from_this());
		m.unlock();
	}

	void ITexture::clear(Color const& clear_color)
	{
		_pendingClear = clear_color;
		//utility::renderer().clear_texture(handle_from_this());
		static mutex m;
		m.lock();
		_toBeCleared.insert(handle_from_this());
		m.unlock();
	}

	ivec2 ITexture::global_to_internal(vec2 const& global) const
	{
		vec2 l = project_to_screen(vec3(global, 0));
		ivec2 result;
		result.x = (s32)(((l.x + 1.0f) / 2.0f) * width());
		result.y = (s32)(((l.y + 1.0f) / 2.0f) * height());
		return result;
	}

	vec2 ITexture::internal_to_global(ivec2 const& internal) const
	{
		vec2 result;
		result.x = ((real)internal.x / width()) * 2.0f - 1.0f;
		result.y = ((real)internal.y / height()) * 2.0f - 1.0f;
		return mouse_pick(result).origin;
	}

	void ITexture::update_textures()
	{
		for (auto& i : _toBeInitialized)
		{
			if (i != nullptr)
			{
				i->init();
			}
		}

		for (auto& i : _toBeResized)
		{
			if (i != nullptr)
			{
				i->on_resize();
				i->_pendingResize = { -1, -1 };
			}
		}

		for (auto& i : _toBeCleared)
		{
			if (i != nullptr)
			{
				i->on_clear();
				i->_pendingClear = { -1, -1, -1, -1 };
			}
		}

		_toBeInitialized.clear();
		_toBeResized.clear();
		_toBeCleared.clear();
	}

	DepthBuffer ITexture::depth_buffer() const
	{
		return DepthBuffer{ .texture = handle_from_this_const() };
	}

	AABB ITexture::area() const
	{
		s32 w = width();
		s32 h = height();
		vec2 ar = { (h < w ? (real)w / (real)h : 1.0f), (w < h ? (real)h / (real)w : 1.0f) };

		AABB result;
		result.left = -ar.x;
		result.right = ar.x;
		result.top = ar.y;
		result.bottom = -ar.y;

		return result;

		//Quad result = Quad(QuadParams{.position = ORIGIN, .size = ar});
		//return result;
	}

#ifdef false
	u8 ITexture::should_cull_triangle(vec3 const& p1, vec3 const& p2, vec3 const& p3)
	{
		vec2 pp1 = project_to_screen(p1);
		vec2 pp2 = project_to_screen(p2);
		vec2 pp3 = project_to_screen(p3);

		// TODO: we had camera flipping in Renderer3D.

		vec2 a = pp2 - pp1;
		vec2 b = pp3 - pp2;

		return (perp_dot(pp2 - pp1, pp3 - pp2) >= 0.0f);
	}
#endif

}
