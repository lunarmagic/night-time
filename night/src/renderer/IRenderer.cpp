
#include "nightpch.h"
#include "IRenderer.h"
#include "window/IWindow.h"
#include <glm/gtc/matrix_transform.hpp>
#include "event/ApplicationEvent.h"
#include "texture/ITexture.h"
#include "material/IShader.h"
#include "material/IMaterial.h"
#include "material/IComputeShader.h"
#include "raycast/raycast.h"
#include "geometry/Quad.h"
#include "text/Text.h"

namespace night
{

	s32 IRenderer::init(const RendererParams& params)
	{
		_window = params.window;
		_eventCallback = params.event_callback;
		clear_color = params.clear_color;
		return 1;
	}

	void IRenderer::update()
	{
		update_impl();
		// TODO: add update event, flush event.
	}

	void IRenderer::present()
	{
		present_impl();

		if (_eventCallback != nullptr)
		{
			RendererPresentedEvent event;
			_eventCallback(event);
		}
	}

	void IRenderer::close()
	{
	}

	void IRenderer::draw_quad(QuadParams const& params)
	{
		draw_quad(Quad(params));
	}

	void IRenderer::activate_material(handle<IMaterial> material)
	{
		_activeMaterial = material;
	}

	handle<IMaterial> IRenderer::active_material() const
	{
		return _activeMaterial;
	}

	void IRenderer::activate_render_target(handle<const ITexture> target)
	{
		ASSERT(target != nullptr); _activeRenderTarget = target;
	}

	handle<IShader> IRenderer::find_shader(const string& id) const
	{
		auto i = _shaders.find(id);

		if (i != _shaders.end())
		{
			return (*i).second;
		}

		return nullptr;
	}

	void IRenderer::destroy_shader(const string& id)
	{
		//static std::mutex s_m;

		auto i = _shaders.find(id);
		if (i != _shaders.end())
		{
			//_textures.erase(i);
			//std::lock_guard<std::mutex> lock(s_m); // TODO: test this
			static mutex m;
			m.lock();
			_shadersPendingDestruction.push_back(i);
			m.unlock();
			TRACE("destroying shader, id: ", id);
		}

		//auto i = _shaders.find(id);
		//if (i != _shaders.end())
		//{
		//	_shaders.erase(i);
		//	TRACE("destroyed shader, id: ", id); // TODO: seperate trace and log, log only in release
		//}
	}

	handle<ITexture> IRenderer::find_texture(const string& id) const
	{
		auto i = _textures.find(id);

		if (i != _textures.end())
		{
			return (*i).second;
		}

		return nullptr;
	}

	void IRenderer::destroy_texture(const string& id)
	{
		//static std::mutex s_m;

		auto i = _textures.find(id);
		if (i != _textures.end())
		{
			//_textures.erase(i);
			//std::lock_guard<std::mutex> lock(s_m); // TODO: test this
			static mutex m;
			m.lock();
			_texturesPendingDestruction.push_back(i);
			m.unlock();
			TRACE("destroying texture, id: ", id);
		}
	}

	void IRenderer::destroy_texture(handle<ITexture> texture)
	{
		for (auto i = _textures.begin(); i != _textures.end(); i++)
		{
			if ((*i).second == (shandle<ITexture>)texture)
			{
				static mutex m;
				m.lock();
				_texturesPendingDestruction.push_back(i);
				m.unlock();
				TRACE("destroying texture via handle, id: ", (*i).first);
			}
		}
	}

	handle<IMaterial> IRenderer::find_material(const string& id) const
	{
		auto i = _materials.find(id);

		if (i != _materials.end())
		{
			return (*i).second;
		}

		return nullptr;
	}

	void IRenderer::destroy_material(const string& id)
	{
		//static std::mutex s_m;

		auto i = _materials.find(id);
		if (i != _materials.end())
		{
			//_textures.erase(i);
			//std::lock_guard<std::mutex> lock(s_m); // TODO: test this
			static mutex m;
			m.lock();
			_materialsPendingDestruction.push_back(i);
			m.unlock();
			TRACE("destroying shader, id: ", id);
		}
		//auto i = _materials.find(id);
		//if (i != _materials.end())
		//{
		//	_materials.erase(i);
		//	TRACE("destroyed material, id: ", id);
		//}
	}

	handle<IComputeShader> IRenderer::find_compute_shader(const string& id) const
	{
		auto i = _computeShaders.find(id);

		if (i != _computeShaders.end())
		{
			return (*i).second;
		}

		return nullptr;
	}

	void IRenderer::destroy_compute_shader(const string& id)
	{
		//static std::mutex s_m;

		auto i = _computeShaders.find(id);
		if (i != _computeShaders.end())
		{
			//_textures.erase(i);
			//std::lock_guard<std::mutex> lock(s_m); // TODO: test this
			static mutex m;
			m.lock();
			_computeShadersPendingDestruction.push_back(i);
			m.unlock();
			TRACE("destroying shader, id: ", id);
		}

		//auto i = _computeShaders.find(id);
		//if (i != _computeShaders.end())
		//{
		//	_computeShaders.erase(i);
		//	TRACE("destroyed compute shader, id: ", id);
		//}
	}

	void IRenderer::cleanup_destroyed_resources()
	{
		for (auto& i : _shadersPendingDestruction)
		{
			ASSERT(i != _shaders.end());
			if (i != _shaders.end())
			{
				TRACE("destroyed texture, id: ", (*i).first);
				_shaders.erase(i); // calls destructor
			}
		}

		for (auto& i : _texturesPendingDestruction)
		{
			ASSERT(i != _textures.end());
			if (i != _textures.end())
			{
				TRACE("destroyed texture, id: ", (*i).first);
				_textures.erase(i); // calls destructor
			}
		}

		for (auto& i : _materialsPendingDestruction)
		{
			ASSERT(i != _materials.end());
			if (i != _materials.end())
			{
				TRACE("destroyed texture, id: ", (*i).first);
				_materials.erase(i); // calls destructor
			}
		}

		for (auto& i : _computeShadersPendingDestruction)
		{
			ASSERT(i != _computeShaders.end());
			if (i != _computeShaders.end())
			{
				TRACE("destroyed texture, id: ", (*i).first);
				_computeShaders.erase(i); // calls destructor
			}
		}

		_shadersPendingDestruction.clear();
		_texturesPendingDestruction.clear();
		_materialsPendingDestruction.clear();
		_computeShadersPendingDestruction.clear();
	}

	void IRenderer::update_mvp()
	{
		Camera const& camera = _defaultRenderTarget->camera();
		glm::mat4 view = glm::lookAt(camera.translation, camera.look_at, camera.up);

		glm::mat4 projection;

		if (camera.type == ECameraType::Perspective)
		{
			projection = glm::perspective(glm::radians(camera.fov), (real)_viewport.z / (real)_viewport.w, camera.near_clip, camera.far_clip);
		}
		else if (camera.type == ECameraType::Orthographic)
		{
			projection = glm::ortho(camera.ortho_region.left, camera.ortho_region.right, camera.ortho_region.bottom, camera.ortho_region.top, camera.near_clip, camera.far_clip);

			s32 w = viewport().z;
			s32 h = viewport().w;
			vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };
			mat4 scale = glm::scale(mat4(1), vec3(ar.x, ar.y, 1.0f)); // we need to manualy scale for ortho

			projection *= scale;
		}
		_mvp = projection * view;
	}

#ifdef false
	Ray IRenderer::mouse_pick(vec2 const& mouse_position)
	{
		vec3 eye = glm::unProject(vec3( mouse_position.x, mouse_position.y, 0.0f ), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		vec3 forward = glm::unProject(vec3(mouse_position.x, mouse_position.y, 1.0f), mat4(1), _mvp, vec4(-1, -1, 2, 2));
		vec3 direction = normalize(forward - eye);
		return {.origin = eye, .direction = direction};
	}

	vec4 IRenderer::project_to_screen(vec3 const& point)
	{
		return vec4(glm::project(point, mat4(1), _mvp, vec4(-1, -1, 2, 2)), 1);
	}

	u8 IRenderer::should_cull_triangle(vec3 const& p1, vec3 const& p2, vec3 const& p3)
	{
		vec2 pp1 = project_to_screen(p1);
		vec2 pp2 = project_to_screen(p2);
		vec2 pp3 = project_to_screen(p3);

		return (perp_dot(pp2 - pp1, pp3 - pp2) >= 0.0f);
	}
#endif

	Quad IRenderer::generate_line_quad(DrawLineParams const& params, handle<const ITexture> target)
	{
		// if p0 == p1, return square of size max width
		Quad area;

		ASSERT(target != nullptr);
		s32 w = target->width();
		s32 h = target->height();

		vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };

		mat4 scale = glm::scale(mat4(1), vec3(ar.x, ar.y, 1.0f)); // TODO: remove this.

		mat4 const& mvp = target->mvp();

		vec3 p1 = glm::project(params.p1, mat4(1), mvp, vec4(-1, -1, 2, 2));
		vec3 p2 = glm::project(params.p2, mat4(1), mvp, vec4(-1, -1, 2, 2));

		vec2 normal = normalize(vec2(p2) / ar - vec2(p1) / ar);
		vec2 perpendicular = vec2(-normal.y, normal.x);
		normal = scale * vec4(normal, 0, 1);
		perpendicular = scale * vec4(perpendicular, 0, 1);

		vec2 p1a = vec2(p1) + (-normal * params.width - perpendicular * params.width);
		vec2 p1b = vec2(p1) + (-normal * params.width + perpendicular * params.width);

		vec2 p2a;
		vec2 p2b;

		if (params.width2 != -1)
		{
			p2a = vec2(p2) + (normal * params.width2 + perpendicular * params.width2);
			p2b = vec2(p2) + (normal * params.width2 - perpendicular * params.width2);
		}
		else
		{
			p2a = vec2(p2) + (normal * params.width + perpendicular * params.width);
			p2b = vec2(p2) + (normal * params.width - perpendicular * params.width);
		}

		area.vertices[0].point = vec4(p1a, 0.0f, 1);
		area.vertices[1].point = vec4(p1b, 0.0f, 1);
		area.vertices[2].point = vec4(p2a, 0.0f, 1);
		area.vertices[3].point = vec4(p2b, 0.0f, 1);
		// TODO: probably want texture coords

		return area;
	}
}