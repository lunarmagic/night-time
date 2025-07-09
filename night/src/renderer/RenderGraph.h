#pragma once

#include "IRenderer.h"
#include "utility.h"
//#include "ref/ref.h"
#include "handle/handle.h"
#include "geometry/Triangle.h"
#include "geometry/Quad.h"
#include "text/Text.h"
#include "log/log.h"
#include "texture/ITexture.h"
#include "material/IMaterial.h"

namespace night
{

	struct TextureUniformData;

	struct NIGHT_API RenderGraph
	{
		struct TriangleToken
		{
			Triangle triangle;
			s32 uniform_index;
			s32 transform_index;
		};

		struct QuadToken
		{
			QuadParams quad;
			s32 uniform_index;
			s32 transform_index;
		};

		struct LineToken
		{
			DrawLineParams line;
			s32 transform_index;
		};

		void draw_triangle(Triangle const& triangle)
		{
			ASSERT(_currentBuffer != nullptr);

			TriangleToken token;
			token.triangle = triangle;
			token.uniform_index = _currentBuffer->current_uniform_index;
			token.transform_index = _currentTransformIndex;

			_currentBuffer->triangles.emplace_back(token);
		}

		void draw_quad(QuadParams const& quad)
		{
			ASSERT(_currentBuffer != nullptr);

			QuadToken token;
			token.quad = quad;
			token.uniform_index = _currentBuffer->current_uniform_index;
			token.transform_index = _currentTransformIndex;

			_currentBuffer->quads.emplace_back(token);
		}

		void draw_quad(Quad const& quad)
		{
			ASSERT(_currentBuffer != nullptr);

			TriangleToken token1;
			token1.uniform_index = _currentBuffer->current_uniform_index;
			token1.transform_index = _currentTransformIndex;
			token1.triangle.vertices[0] = quad.vertices[0];
			token1.triangle.vertices[1] = quad.vertices[1];
			token1.triangle.vertices[2] = quad.vertices[2];

			TriangleToken token2;
			token2.uniform_index = _currentBuffer->current_uniform_index;
			token2.transform_index = _currentTransformIndex;
			token2.triangle.vertices[0] = quad.vertices[2];
			token2.triangle.vertices[1] = quad.vertices[3];
			token2.triangle.vertices[2] = quad.vertices[0];

			_currentBuffer->triangles.emplace_back(token1);
			_currentBuffer->triangles.emplace_back(token2);
		}

		void draw_point(const vec3& point, const Color& color = BLACK)
		{
			draw_line(point, point + UP * RENDERER_POINT_DEFAULT_HEIGHT, color, RENDERER_POINT_DEFAULT_WIDTH);
		};

		void draw_point(const vec2& point, const Color& color = BLACK)
		{
			draw_point(vec3(point, 0), color);
		}

		void draw_line(DrawLineParams const& params)
		{
			ASSERT(_currentBuffer != nullptr);

			LineToken token;
			token.line = params;
			token.transform_index = _currentTransformIndex;

			_currentBuffer->lines.emplace_back(token);
		};

		void draw_line(vec2 const& p1, vec2 const& p2, Color const& color = BLACK, real width = RENDERER_LINE_DEFAULT_WIDTH)
		{
			draw_line(DrawLineParams{ .p1 = vec3(p1, 0), .p2 = vec3(p2, 0), .color = color, .width = width });
		}

		void draw_line(vec3 const& p1, vec3 const& p2, Color const& color = BLACK, real width = RENDERER_LINE_DEFAULT_WIDTH)
		{
			draw_line(DrawLineParams{ .p1 = p1, .p2 = p2, .color = color, .width = width });
		}

		void draw_line(vec4 const& p1, vec4 const& p2, Color const& color = BLACK, real width = RENDERER_LINE_DEFAULT_WIDTH)
		{
			draw_line(DrawLineParams{ .p1 = (vec3)p1, .p2 = (vec3)p2, .color = color, .width = width });
		}

#if 0
		void draw_text(Text const& text)
		{
			ASSERT(_currentBuffer != nullptr);
			_currentBuffer->texts.emplace_back(text);
		}
#endif

		// TODO: meshes will be there own node with variable num of materials, bones, animations
		// TODO: add way to append vertices directly to renderer, no iterating


		vector<TriangleToken> const& triangles() const { ASSERT(_currentBuffer != nullptr); return _currentBuffer->triangles; }
		vector<LineToken> const& lines() const { ASSERT(_currentBuffer != nullptr); return _currentBuffer->lines; } // TODO: use shared vector for all lines across rendergraphs
		vector<QuadToken> const& quads() const { ASSERT(_currentBuffer != nullptr); return _currentBuffer->quads; }

		void append_uniform_buffer(vector<u8> const& buffer)
		{
			ASSERT(buffer.size() != 0);
			ASSERT(_currentBuffer != nullptr);
			//ASSERT(buffer.size() == 0 || _currentBuffer->uniform_storage.size() % buffer.size() == 0);
			ASSERT(_currentBuffer->uniform_storage.size() % buffer.size() == 0);
			
			for (const auto& i : buffer)
			{
				_currentBuffer->uniform_storage.emplace_back(i); // TODO: optimize
			}

			_currentBuffer->current_uniform_index++;
		}

		template<typename T>
		inline void append_uniform_buffer(T const& t)
		{
			ASSERT(_currentBuffer != nullptr)
			ASSERT(_currentBuffer->uniform_storage.size() % sizeof(T) == 0);

			for (s32 i = 0; i < sizeof(T); i++)
			{
				_currentBuffer->uniform_storage.emplace_back(((const u8*)&t)[i]);
			}

			_currentBuffer->current_uniform_index++;

			//_activeUniformBuffer = vector<u8>(sizeof(T));
			//memcpy(_activeUniformBuffer.data(), (void*)&t, sizeof(T));
		}

		void append_transform(mat4 const& transform)
		{
			_transformStorage.emplace_back(transform);
			_currentTransformIndex++;
		}

		vector<u8> const& uniform_storage() const { ASSERT(_currentBuffer != nullptr); return _currentBuffer->uniform_storage; }
		vector<mat4> const& transform_storage() const { ASSERT(_currentBuffer != nullptr); return _transformStorage; }

		//void current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, TextureUniformData const& texture);
		void current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, vector<TextureUniformData> const& textures);

		//void current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, handle<const ITexture> texture);
		void current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, vector<handle<const ITexture>> const& textures); // TODO: remove

		void current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, std::nullptr_t null);

		template<typename... Textures>
		void current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, Textures... textures);

		// only storage that is not a uniform size needs to be in the buffer object
		vector<mat4> _transformStorage;
		s32 _currentTransformIndex = -1;

		struct Buffer
		{
			vector<TriangleToken> triangles;
			vector<QuadToken> quads;
			vector<LineToken> lines;

			//vector<Text> texts;

			s32 current_uniform_index{ -1 }; // TODO: may want different node graphs per graphics api
			//s32 current_transform_index{ -1 };
			vector<u8> uniform_storage;
			//vector<mat4> transform_storage;
		};

		Buffer* current_buffer() { return _currentBuffer; }
		void current_buffer(Buffer* buffer) { _currentBuffer = buffer; }

		map < // TODO: may be able to use set
			shandle<const ITexture>, // render targets
			map<
			shandle<IMaterial>, // materials
			map<
			vector<TextureUniformData>, // textures
			Buffer
			>
			>
		> const& sorted_buffers() const { return _sortedBuffers; }

		handle<const ITexture> const& current_render_target() { return _currentRenderTarget; }
		handle<IMaterial> const& current_material() { return _currentMaterial; }

		void clear()
		{
			_sortedBuffers.clear();
			_transformStorage.clear();
			_currentTransformIndex = -1;
			_currentBuffer = nullptr;
			_currentRenderTarget = nullptr;
			_currentMaterial = nullptr;
		}

		u8 empty()
		{
			return _sortedBuffers.empty();
		}

		// TODO: use umap
		map < // TODO: may be able to use set
			shandle<const ITexture>, // render targets
			map<
				shandle<IMaterial>, // materials
				map<
					vector<TextureUniformData>, // textures
					Buffer
				>
			>
		> _sortedBuffers;

		Buffer* _currentBuffer{ nullptr };
		handle<const ITexture> _currentRenderTarget{ nullptr };
		handle<IMaterial> _currentMaterial{ nullptr };
	};

	template<typename ...Textures>
	inline void RenderGraph::current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, Textures ...textures)
	{
		if (render_target == nullptr)
		{
			render_target = utility::renderer().default_render_target();
			ASSERT(render_target != nullptr);
		}

		_currentRenderTarget = render_target;
		_currentMaterial = material;

		vector<TextureUniformData> s_textures;

		([&]
			{
				if constexpr (std::is_same_v<decltype(textures), DepthBuffer>)
				{
					TextureUniformData data;
					ASSERT(textures.texture != nullptr);
					data.texture = textures.texture;
					data.sample_depth_buffer = true;
					s_textures.push_back(data);
				}
				else
				{
					TextureUniformData data;
					ASSERT(textures != nullptr);
					data.texture = textures;
					data.sample_depth_buffer = false;
					s_textures.push_back(data);
				}
			} (), ...);

		_currentBuffer = &_sortedBuffers[render_target][material][s_textures];
	}

}