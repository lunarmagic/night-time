#pragma once

#include "core.h"
#include "color/Color.h"
#include "math/math.h"
#include "ref/ref.h"
#include "handle/handle.h"
#include "camera/Camera.h"
#include "material/IMaterial.h"

namespace night
{
	struct IWindow;
	struct INode;
	struct RenderGraph;
	struct Event;
	struct Quad;
	struct QuadParams;
	struct Text;
	struct ITexture;
	struct TextureParams;
	struct IShader;
	//struct IMaterial;
	struct IComputeShader;
	struct ShaderParams;
	struct MaterialParams;
	struct ComputeShaderParams;
	struct Ray;
	//struct TextureUniformData;

	struct RendererParams
	{
		IWindow* window;
		Color clear_color = WHITE;

		function<void(Event&)> event_callback{nullptr};
	};

#define RENDERER_LINE_DEFAULT_WIDTH 0.005f
#define RENDERER_POINT_DEFAULT_WIDTH 0.0075f
#define RENDERER_POINT_DEFAULT_HEIGHT 0.05f

#define RENDERER_DEFAULT_RENDER_TARGET_NEAR_CLIP NIGHT_CAMERA_DEFAULT_NEAR_CLIP
#define RENDERER_DEFAULT_RENDER_TARGET_FAR_CLIP NIGHT_CAMERA_DEFAULT_FAR_CLIP * 2.0f

	struct DrawLineParams
	{
		vec3 p1{ 0 };
		vec3 p2{ 0 };
		Color color{ BLACK };
		Color color2{ Color(-1, -1, -1, -1) };
		real width{ RENDERER_LINE_DEFAULT_WIDTH };
		real width2{ -1 };
	};

	// TODO: add compute shader interface
	struct NIGHT_API IRenderer
	{
		virtual s32 init(const RendererParams& params);

		virtual void update_resources() = 0;

		void update();
		void present();
		virtual void close();
		virtual void draw_pixel(ivec2 const& internal_coord, Color const& color) = 0;

		virtual void draw_point(const vec3& point, const Color& color) = 0;
		void draw_point(const vec2& point, const Color& color) { draw_point(vec3(point, 0), color); }

		virtual void draw_line(DrawLineParams const& params) = 0;

		void draw_line(vec2 const& p1, vec2 const& p2, Color const& color = BLACK, real width = RENDERER_LINE_DEFAULT_WIDTH)
		{
			draw_line(DrawLineParams{.p1 = vec3(p1, 0), .p2 = vec3(p2, 0), .color = color, .width = width});
		}

		void draw_line(vec3 const& p1, vec3 const& p2, Color const& color = BLACK, real width = RENDERER_LINE_DEFAULT_WIDTH)
		{
			draw_line(DrawLineParams{ .p1 = p1, .p2 = p2, .color = color, .width = width });
		}

		void draw_line(vec4 const& p1, vec4 const& p2, Color const& color = BLACK, real width = RENDERER_LINE_DEFAULT_WIDTH)
		{
			draw_line(DrawLineParams{ .p1 = (vec3)p1, .p2 = (vec3)p2, .color = color, .width = width });
		}

		virtual void draw_quad(Quad const& quad) = 0;
		void draw_quad(QuadParams const& params);

		virtual void draw_text(Text const& text) = 0;

		virtual void flush() = 0;
		virtual void flush_render_graph(RenderGraph const& graph) = 0;

		void activate_material(handle<IMaterial> material);
		void deactivate_material() { _activeMaterial = nullptr; }
		handle<IMaterial> active_material() const;

		template<typename... Textures>
		void activate_textures(Textures&&... textures);
		vector<TextureUniformData>& active_textures() { return _activeTextures; } // TODO: this function should not be exposed to the api.
		void deactivate_textures() { _activeTextures.clear(); }

		template<typename T>
		void activate_uniform_storage(T const& t);
		void deactivate_uniform_storage() { _activeUniformBuffer.clear(); }

		void activate_render_target(handle<const ITexture> target);
		void deactivate_render_target() { _activeRenderTarget = _defaultRenderTarget; }
		handle<const ITexture> active_render_target() const { return _activeRenderTarget; }

		void activate_transform(mat4 const transform) { _activeTransform = transform; }
		void deactivate_transform() { _activeTransform = mat4(1); }
		mat4 const& active_transform() const { return _activeTransform; }

		const mat4& mvp() const { return _mvp; }

		Color clear_color{ WHITE };

		virtual void on_window_resize(u32 width, u32 height) = 0;

		virtual handle<IShader> create_shader(const string& id, const ShaderParams& params) = 0;
		handle<IShader> find_shader(const string& id) const;
		void destroy_shader(const string& id);

		virtual handle<ITexture> create_texture(const string& id, const TextureParams& params) = 0;
		handle<ITexture> find_texture(const string& id) const;
		void destroy_texture(const string& id);
		void destroy_texture(handle<ITexture> texture);

		virtual handle<IMaterial> create_material(const string& id, const MaterialParams& params) = 0;
		handle<IMaterial> find_material(const string& id) const;
		void destroy_material(const string& id);

		virtual handle<IComputeShader> create_compute_shader(const string& id, const ComputeShaderParams& params) = 0;
		handle<IComputeShader> find_compute_shader(const string& id) const;
		void destroy_compute_shader(const string& id);

		void cleanup_destroyed_resources();

		void reset_state()
		{
			_activeMaterial = nullptr;
			_activeTextures.clear();
			_activeUniformBuffer.clear();
			_activeRenderTarget = _defaultRenderTarget;
		}

		s32 width() const { return _viewport.z; }
		s32 height() const { return _viewport.w; }

		handle<const ITexture> default_render_target() const { return _defaultRenderTarget; }

		static Quad generate_line_quad(DrawLineParams const& params, handle<const ITexture> target);

	protected:

		virtual void update_impl() = 0;
		virtual void present_impl() = 0;

		IWindow* window() const { return _window; }

		map<string, shandle<IShader>> _shaders;
		map<string, shandle<ITexture>> _textures;
		map<string, shandle<IMaterial>> _materials;
		map<string, shandle<IComputeShader>> _computeShaders;

		// TODO: move to private.
		// active material should be set to default after every frame, same for textures and uniform buffer.
		shandle<const ITexture> _activeRenderTarget{ nullptr };
		shandle<IMaterial> _activeMaterial{ nullptr };
		vector<TextureUniformData> _activeTextures{}; // TODO: make textures const
		vector<u8> _activeUniformBuffer{};
		mat4 _activeTransform{ mat4(1) };
		// TODO: add u8 _hasStateChanged so we can avoid looking up the corresponding draw call.

		shandle<ITexture> _defaultRenderTarget{nullptr};

		void viewport(ivec4 vp) { _viewport = vp; update_mvp(); }
		ivec4 const& viewport() const { return _viewport; }
		
	private:

		IWindow* _window;
		mat4 _mvp{ mat4(1)};

		vector<map<string, shandle<IShader>>::iterator> _shadersPendingDestruction; // TODO: use uset
		vector<map<string, shandle<ITexture>>::iterator> _texturesPendingDestruction; // TODO: use uset
		vector<map<string, shandle<IMaterial>>::iterator> _materialsPendingDestruction; // TODO: use uset
		vector<map<string, shandle<IComputeShader>>::iterator> _computeShadersPendingDestruction; // TODO: use uset

		ivec4 _viewport{ ivec4(0) };
		void update_mvp();

		function<void(Event&)> _eventCallback{nullptr};
	};

	template<typename ...Textures>
	inline void IRenderer::activate_textures(Textures && ...textures)
	{
		_activeTextures.clear();
		for (const auto i : { textures... })
		{
			_activeTextures.emplace_back(TextureUniformData{ .texture = i.ptr().lock(), .sample_depth_buffer = false });
		}

		//_activeTextures.clear();
		//for (const auto i : { textures... })
		//{
		//	_activeTextures.emplace_back(i.ptr().lock());
		//}
	}

	template<typename T>
	inline void IRenderer::activate_uniform_storage(T const& t)
	{
		_activeUniformBuffer = vector<u8>(sizeof(T));
		memcpy(_activeUniformBuffer.data(), (void*)&t, sizeof(T));
	}

}