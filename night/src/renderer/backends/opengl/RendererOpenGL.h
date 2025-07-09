#pragma once

#include "core.h"
#include "renderer/IRenderer.h"
#include "IndexBufferOpenGL.h"
#include "ShaderOpenGL.h"
#include "VertexArrayOpenGL.h"
#include "VertexBufferOpenGL.h"
#include "VertexBufferLayoutOpenGL.h"
#include "ShaderStorageBufferOpenGL.h"
//#include "MaterialOpenGL.h"
#include "TextureOpenGL.h"

#define OPEN_GL_MAX_QUAD_COUNT 30000
#define OPEN_GL_MAX_VERTEX_COUNT OPEN_GL_MAX_QUAD_COUNT * 4
#define OPEN_GL_MAX_INDEX_COUNT OPEN_GL_MAX_QUAD_COUNT * 6

#define NIGHT_RENDERER_OPENGL_DEFAULT_DEPTH_PEEL_COUNT 3 // TODO: figure out why performance drops after 3 depth peels
#define NIGHT_RENDERER_OPENGL_DEFAULT_DEPTH_PEEL_RESOLUTION ivec2(4096, 2160) // TODO: load from file
#define NIGHT_RENDERER_OPENGL_DEFAULT_VSYNC 1
#define NIGHT_RENDERER_OPENGL_UNIFORM_BUFFER_BINDING 0
#define NIGHT_RENDERER_OPENGL_TRANSFORM_BUFFER_BINDING 1

struct SDL_Renderer;
struct SDL_Window;
struct SDL_GLContextState;

namespace night
{

	struct MaterialOpenGL;
	struct TextureUniformData;
	struct TextureOpenGL;

	struct VertexOpenGL
	{
		Vertex vertex;
		s32 uniform_index;
		s32 transform_index;
	};

	struct DrawCallOpenGL
	{
		vector<VertexOpenGL> vertices;
		vector<u8> storage;
		//vector<mat4> transforms;
	};

	struct NIGHT_API RendererOpenGL : public IRenderer
	{
		virtual s32 init(const RendererParams& params) override;

		virtual void update_resources() override;

		virtual void close() override;
		virtual void draw_pixel(ivec2 const& internal_coord, Color const& color) override;
		virtual void draw_point(const vec3& point, const Color& color) override;
		virtual void draw_quad(Quad const& quad) override;
		virtual void draw_line(DrawLineParams const& params) override;
		virtual void draw_text(Text const& text) override;

		virtual void flush();
		virtual void flush_render_graph(RenderGraph const& graph) override;

		virtual void on_window_resize(u32 width, u32 height) override;

		SDL_GLContextState* const& context() { return _context; }

		virtual handle<IShader> create_shader(const string& id, const ShaderParams& params) override;
		virtual handle<ITexture> create_texture(const string& id, const TextureParams& params) override;
		virtual handle<IMaterial> create_material(const string& id, const MaterialParams& params) override;
		virtual handle<IComputeShader> create_compute_shader(const string& id, const ComputeShaderParams& params) override;

	protected:

		virtual void update_impl() override;
		virtual void present_impl() override;

	private:

		SDL_GLContextState* _context;
		VertexArrayOpenGL _vao;
		VertexBufferOpenGL<VertexOpenGL> _vbo;
		VertexBufferLayoutOpenGL _vblo;
		IndexBufferOpenGL _ibo;
		ShaderStorageBufferOpenGL _uniformBuffer;
		ShaderStorageBufferOpenGL _transformBuffer;

		vector<handle<TextureOpenGL>> _depthPeels;
		
		handle<ShaderOpenGL> _defaultShader;
		handle<MaterialOpenGL> _defaultMaterial;

		handle<ShaderOpenGL> _defaultLineShader;
		handle<MaterialOpenGL> _defaultLineMaterial;

		handle<ShaderOpenGL> _depthPeelShader;
		handle<MaterialOpenGL> _depthPeelMaterial;

		std::multimap< // TODO: fix night::map template.
			shandle<const ITexture>, // render targets
				map<
				shandle<MaterialOpenGL>, // materials
					map<
						vector<TextureUniformData>, // textures
						DrawCallOpenGL>
			>, CompareITextureSHandleByRenderFlushPriority
		> _drawCalls;
		// TODO: store a prev dc iterator so we don't have to traverse the map every draw call

		vector<mat4> _transformStorage;

		s32 _depthPeelCount = NIGHT_RENDERER_OPENGL_DEFAULT_DEPTH_PEEL_COUNT;
		ivec2 _depthPeelResolution = NIGHT_RENDERER_OPENGL_DEFAULT_DEPTH_PEEL_RESOLUTION;

		SDL_Window* _sdlWindow{ nullptr };
	};

}