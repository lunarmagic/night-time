
#include "nightpch.h"
#include "RendererOpenGL.h"
#include "SDL3/SDL.h"
#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <gl\glu.h>
#include "window/backends/sdl/WindowSDL.h"
#include "math/math.h"
#include <glm/gtc/matrix_transform.hpp>
//#include "Renderer3D/Camera.h"
//#include "node/INode.h"
//#include "node/NodeRenderable.h"
//#include "node/NodeRenderTarget.h"
#include "renderer/RenderGraph.h"
#include "text/Text.h"
#include "profiler/Profiler.h"
#include "GLCall.h"
//#include "GL/glew.h"
#include "ComputeShaderOpenGL.h"
#include "MaterialOpenGL.h"
#include "texture/Surface.h"
#include "raycast/raycast.h"
//#include "TextureOpenGL.h"

#define RENDERER_DEPTH_PEEL_COUNT 3 // TODO: figure out why performance drops after 3 depth peels
#define RENDERER_DEPTH_PEEL_RESOLUTION ivec2(4096, 2160) // TODO: load from file

namespace night
{
	//Camera _test_camera; // TODO: remove

	static const string _depth_peel_vs =
		R"(
		#version 330 core

		layout(location = 0) in vec4 a_position;
		layout(location = 2) in vec2 a_coord;
		
		out vec4 v_color;
		out vec2 v_coord;
			
		void main()
		{
			gl_Position = a_position;
			v_coord = a_coord;
		};
	)";

	static const string _depth_peel_fs =
		R"(
		#version 330 core
		
		layout(location = 0) out vec4 o_color;

		in vec2 v_coord;
		uniform sampler2D u_texture;

		void main()
		{
			ivec2 size = textureSize(u_texture, 0);
			vec2 uv = v_coord * size;
			vec2 seam = floor(uv + 0.5);
			vec2 dudv = fwidth(uv);
			uv = seam + clamp((uv - seam) / dudv, -0.5, 0.5);
			uv /= size;
			
			o_color = texture(u_texture, uv);
		};
	)";

	static const string _default_vs =
		R"(
		#version 330 core

		layout(location = 0) in vec4 a_position;
		layout(location = 1) in vec4 a_color;
		layout(location = 2) in vec2 a_coord;
		
		uniform mat4 u_mvp;
		
		out vec2 v_coord;
		out vec4 v_color;
			
		void main()
		{
			gl_Position = u_mvp * a_position;
			//gl_Position = a_position;
			v_coord = a_coord;
			v_color = a_color;
		};
	)";

	// TODO: default fragment shader has 1 texture slot, if no texture is bound we should not sample texture
	static const string _default_fs =
		R"(
		#version 330 core
		
		layout(location = 0) out vec4 o_color;

		in vec4 v_color;
		in vec2 v_coord;
		uniform sampler2D u_texture;

		uniform int u_has_texture;

		uniform sampler2D u_prev_depth; // depth peeling
		uniform int u_depth_peel; // TODO: may not need this
		
		void main()
		{
			if(u_depth_peel == 1)
			{
				vec4 prev_depth = texelFetch(u_prev_depth, ivec2(gl_FragCoord.xy), 0);
	
				if(gl_FragCoord.z <= prev_depth.r)
				{
					discard;
				}
			}

			if(u_has_texture == 0)
			{
				o_color = v_color;
			}
			else
			{
				ivec2 size = textureSize(u_texture, 0);
				vec2 uv = v_coord * size;
				vec2 seam = floor(uv + 0.5);
				vec2 dudv = fwidth(uv);
				uv = seam + clamp((uv - seam) / dudv, -0.5, 0.5);
				uv /= size;
				uv.y = -uv.y;

				o_color = texture(u_texture, uv);
				o_color *= v_color;
			}

			if(o_color.a < 0.001f)
			{
				discard;
			}
		};
	)";



	static const string _default_line_vs =
		R"(
		#version 330 core

		layout(location = 0) in vec4 a_position;
		layout(location = 1) in vec4 a_color;
		layout(location = 3) in float a_uniform_index;

		out vec4 v_color;
		out flat int v_uniform_index;
			
		void main()
		{
			gl_Position = a_position;
			v_color = a_color;
			v_uniform_index = int(a_uniform_index);
		};
	)";

	// TODO: make anti aliasing parameters into uniforms, add getter for line material.
	// TODO: optimize distance
	static const string _default_line_fs =
		R"(
		#version 330 core

		struct Line
		{
			float[3] p1;
			float[3] p2;
			float[4] color1;
			float[4] color2;
			float width1;
			float width2;
		};

		struct Distance
		{
			float distance;
			float t;
		};

		layout(std430, binding = 0) buffer ssbo
		{
			Line u_lines[];
		};
		
		layout(location = 0) out vec4 o_color;

		in vec4 v_color;
		in flat int v_uniform_index;

		uniform vec2 u_viewport_size;

		uniform sampler2D u_prev_depth; // depth peeling
		uniform int u_depth_peel; // TODO: may not need this

		Distance distance_to_line(vec2 point, vec2 line_a, vec2 line_b)
		{
			// TODO: fix this function
			vec2 ab = line_b - line_a;
			float t = dot(point - line_a, ab) / dot(ab, ab);
			t = min(max(t, 0.0f), 1.0f);
			vec2 projected_point = line_a + ab * t;
			float dist = distance(point, projected_point);

			Distance result;
			result.distance = dist;
			result.t = t;
			return result;
		};
		
		void main()
		{
			Line line = u_lines[v_uniform_index];

			vec2 p1 = vec2(line.p1[0], line.p1[1]);
			vec2 p2 = vec2(line.p2[0], line.p2[1]);
			float depth1 = line.p1[2];
			float depth2 = line.p2[2];
			float width1 = line.width1;
			float width2 = line.width2;
			vec4 color1 = vec4(line.color1[0],line.color1[1],line.color1[2],line.color1[3]);
			vec4 color2 = vec4(line.color2[0],line.color2[1],line.color2[2],line.color2[3]);
			
			// fragment point in normalized screen coordinates.
			vec2 point = vec2(gl_FragCoord) / u_viewport_size;
			point *= 2;
			point -= 1;

			float w = u_viewport_size.x;
			float h = u_viewport_size.y;
			
			float wr;
			float hr;
			
			if(h < w)
			{
				wr = float(h) / float(w);
				hr = 1.0f;
			}
			else
			{
				wr = 1.0f;
				hr = float(w) / float(h);
			}
			
			vec2 aspect = vec2(wr, hr);
			Distance dist = distance_to_line(point / aspect, p1 / aspect, p2 / aspect);

			float width = width1 + (width2 - width1) * dist.t;

			// TODO: apply anti aliasing, optimize
			if(dist.distance > width)
			{
				discard;
			}

			float depth = depth1 + (depth2 - depth1) * dist.t;

			if(u_depth_peel == 1) // it is slow to do depth peeling this late into the function.
			{
				vec4 prev_depth = texelFetch(u_prev_depth, ivec2(gl_FragCoord.xy), 0);
	
				if(depth <= prev_depth.r)
				{
					discard;
				}
			}

			gl_FragDepth = depth;

			vec4 color = color1 + (color2 - color1) * dist.t;

			//float dist_to_width = width - dist.distance;
			//float min_res = min(w, h);
			//float dist_screen = dist_to_width * min_res;
			//float aa_pixel_range = 3.0f;
			//float clamped = max(aa_pixel_range - dist_screen, 0.0f);
			//float aa_t = 1.0f - clamped / aa_pixel_range;

			//color.a *= aa_t;

			o_color = color;

			if(o_color.a < 0.001f)
			{
				discard;
			}
		};
	)";

	static inline uint32_t* _getIndices()
	{
		static uint32_t indices[OPEN_GL_MAX_INDEX_COUNT];
		//uint32_t offset = 0;

		//for (size_t i = 0; i < OPEN_GL_MAX_INDEX_COUNT; i += 6)
		//{
		//	indices[i + 0] = 0 + offset;
		//	indices[i + 1] = 1 + offset;
		//	indices[i + 2] = 2 + offset;

		//	indices[i + 3] = 2 + offset;
		//	indices[i + 4] = 3 + offset;
		//	indices[i + 5] = 0 + offset;

		//	offset += 4;
		//}

		for (size_t i = 0; i < OPEN_GL_MAX_INDEX_COUNT; i++)
		{
			indices[i] = (u32)i;
		}

		return indices;
	}

	s32 RendererOpenGL::init(const RendererParams& params)
	{
		__super::init(params);

		WindowSDL* sdl_window = (WindowSDL*)params.window;
		_sdlWindow = sdl_window->sdl_window();

		//ComputeShaderOpenGL test;

		//test.load("a");
		//test.clean();

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		_context = SDL_GL_CreateContext(_sdlWindow);
		if (_context == NULL)
		{
			ERROR("OpenGL context could not be created! SDL Error: ", SDL_GetError());
			return -1;
		}

		SDL_GL_MakeCurrent(_sdlWindow, _context);

		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();

		if (glewError != GLEW_OK)
		{
			ERROR("Failed initializing GLEW! ", glewGetErrorString(glewError));
		}

		//if (SDL_GL_SetSwapInterval(0) == false)
		//{
		//	WARNING("Unable to set VSync! SDL Error: ", SDL_GetError());
		//}

		// TODO: make vsync setting
		if (SDL_GL_SetSwapInterval(0) == false)
		{
			WARNING("Unable to set VSync! SDL Error: ", SDL_GetError());
		}

		_ibo.init(_getIndices(), OPEN_GL_MAX_INDEX_COUNT);

		_vblo.push<real>(4); // vertices
		_vblo.push<real>(4); // color
		_vblo.push<real>(2); // texture coords
		_vblo.push<u32>(1); // uniform index

		_vao.init();
		_vbo.init(nullptr, 0);
		_vao.addBuffer(_vbo, _vblo);

		_ssbo.init(0);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glDepthFunc(GL_LESS));

		// TODO: get default materials/shaders out of managers, set uniforms
		_defaultShader = create_shader("Default", ShaderParams{ _default_vs, _default_fs, true });
		_defaultMaterial = create_material("Default", { .shader = _defaultShader });
		ASSERT(_defaultMaterial != nullptr);

		_defaultLineShader = create_shader("Default Line", ShaderParams{ _default_line_vs, _default_line_fs, true });
		_defaultLineMaterial = create_material("Default Line", { .shader = _defaultLineShader });

		_depthPeelShader = create_shader("Depth Peel", ShaderParams{ _depth_peel_vs, _depth_peel_fs, true });
		_depthPeelMaterial = create_material("Depth Peel", { .shader = _depthPeelShader });

		ivec4 viewport = { 0, 0, params.window->width(), params.window->height() };
		GLCall(glViewport(viewport.x, viewport.y, viewport.z, viewport.w));

		{
			SurfaceParams sf_params;
			sf_params.width = RENDERER_DEPTH_PEEL_RESOLUTION.x;
			sf_params.height = RENDERER_DEPTH_PEEL_RESOLUTION.y;
			sf_params.fill_color = params.clear_color;
			sf_params.fill_color.a = 0;

			Surface surface(sf_params);

			// TODO: load depth peel count from file.
			for (s32 i = 0; i < RENDERER_DEPTH_PEEL_COUNT; i++)
			{
				auto& dp = _depthPeels.emplace_back(create_texture("Depth Peel #" + to_string(i), { .surface = &surface }));
				ASSERT(dp != nullptr);
			}
		}

		{
			SurfaceParams sf_params;
			sf_params.width = viewport.z;
			sf_params.height = viewport.w;
			sf_params.fill_color = params.clear_color;

			Surface surface(sf_params);

			_defaultRenderTarget = create_texture("Default Render Target", { .surface = &surface });
			ASSERT(_defaultRenderTarget != nullptr);
			_defaultRenderTarget->render_flush_priority(INFINITY);

			Camera camera;
			camera.translation = FORWARD * 100.0f;
			camera.look_at = ORIGIN;
			camera.up = UP;
			camera.near_clip = RENDERER_DEFAULT_RENDER_TARGET_NEAR_CLIP;
			camera.far_clip = RENDERER_DEFAULT_RENDER_TARGET_FAR_CLIP;

			camera.type = ECameraType::Orthographic;
			camera.ortho_region = { .left = -1, .right = 1, .top = 1, .bottom = -1 };
			_defaultRenderTarget->camera(camera);
		}

		this->viewport(viewport);

		clear_color = params.clear_color;

		return 1;
	}

	void RendererOpenGL::update_impl()
	{
		flush();

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); // must not have render target bound.
		GLCall(glViewport(viewport().x, viewport().y, viewport().z, viewport().w));

		GLCall(glDisable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

		Quad quad(QuadParams{ .position = {0, 0, 0}, .size = { 1, 1 } });
		quad.vertices[0].texture_coord = vec2(0, 0);
		quad.vertices[1].texture_coord = vec2(1, 0);
		quad.vertices[2].texture_coord = { 1, 1 };
		quad.vertices[3].texture_coord = vec2(0, 1);

		VertexOpenGL vertices[6];
		vertices[0] = { quad.vertices[0], 0 };
		vertices[1] = { quad.vertices[1], 0 };
		vertices[2] = { quad.vertices[2], 0 };
		vertices[3] = { quad.vertices[2], 0 };
		vertices[4] = { quad.vertices[3], 0 };
		vertices[5] = { quad.vertices[0], 0 };

		_depthPeelMaterial->uniform("u_texture", _defaultRenderTarget);
		_depthPeelMaterial->bind();
		_vbo.data(&vertices, 6);
		GLCall(glDrawElements(GL_TRIANGLES, _vbo.count(), GL_UNSIGNED_INT, nullptr)); // TODO: this crashes if render graph is empty
	}

	void RendererOpenGL::present_impl()
	{
		NIGHT_PROFILER_START_TIMER("Renderer Present");
		SDL_GL_SwapWindow(_sdlWindow);

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); // TODO: remove
		GLCall(glViewport(viewport().x, viewport().y, viewport().z, viewport().w));

		Color const& cc = clear_color;
		GLCall(glClearColor(cc.r, cc.g, cc.b, cc.a));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, handle<TextureOpenGL>(_defaultRenderTarget)->fbo()));
		GLCall(glViewport(0, 0, _defaultRenderTarget->width(), _defaultRenderTarget->height()));

		GLCall(glClearColor(cc.r, cc.g, cc.b, cc.a));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		reset_state();

		NIGHT_PROFILER_STOP_TIMER("Renderer Present");
	}

	void RendererOpenGL::close()
	{
		_ssbo.clean();
		_ibo.clean();
		_vbo.clean();
		_vblo.clean();
		_vao.clean();

		for (auto& i : materials())
		{
			if (i.second != nullptr)
			{
				i.second->clean();
			}
		}

		for (auto& i : shaders())
		{
			if (i.second != nullptr)
			{
				i.second->clean();
			}
		}

		for (auto& i : compute_shaders())
		{
			if (i.second != nullptr)
			{
				i.second->clean();
			}
		}

		for (auto& i : textures())
		{
			if (i.second != nullptr)
			{
				i.second->clean();
			}
		}

		SDL_GL_DestroyContext(_context);
	}

	void RendererOpenGL::draw_pixel(ivec2 const& internal_coord, Color const& color)
	{
		// TODO: impl
	}

	void RendererOpenGL::draw_point(const vec3& point, const Color& color)
	{
		draw_line(DrawLineParams{ .p1 = point, .p2 = point + UP * RENDERER_POINT_DEFAULT_HEIGHT, .color = color, .width = RENDERER_POINT_DEFAULT_WIDTH });
	}

	void RendererOpenGL::draw_quad(Quad const& quad)
	{
		auto fn = [&](handle<IMaterial> active_material)
		{
			auto f = _drawCalls.find(_activeRenderTarget);
			if (f == _drawCalls.end())
			{
				f = _drawCalls.insert({ _activeRenderTarget, {} });
			}

			auto& draw_call = (*f).second[active_material][_activeTextures];

			auto mat = handle<MaterialOpenGL>(active_material);
			ASSERT(_activeUniformBuffer.size() == mat->storage()); // size of struct and ssbo do not match

			u32 uniform_index = mat->storage() == 0 ? 0 : (u32)(draw_call.storage.size() / mat->storage());

			draw_call.vertices.push_back({ quad.vertices[0], uniform_index }); // TODO: optimize this
			draw_call.vertices.push_back({ quad.vertices[1], uniform_index });
			draw_call.vertices.push_back({ quad.vertices[2], uniform_index });
			draw_call.vertices.push_back({ quad.vertices[2], uniform_index });
			draw_call.vertices.push_back({ quad.vertices[3], uniform_index });
			draw_call.vertices.push_back({ quad.vertices[0], uniform_index });

			for (const auto& i : _activeUniformBuffer)
			{
				draw_call.storage.emplace_back(i);
			}
		};

		if (_activeMaterial == nullptr)
		{
			ASSERT(_defaultMaterial != nullptr);
			ASSERT(_defaultMaterial->storage() == 0); // default material has no uniform buffer
			fn(_defaultMaterial);
		}
		else
		{
			fn(_activeMaterial);
		}
	}

	vec3 _project_point_to_view_plane(const vec4& point, real view_angle)
	{
		vec3 origin = { 0.0f, 0.0f, 0.0f };
		auto rc = raycast::plane(point, (BACKWARDS * view_angle) - vec3(point), origin, FORWARD);
		vec3 contact = rc.contact(point, (BACKWARDS * view_angle) - vec3(point));
		return { contact.x, contact.y, 0.0f };
	}

	void RendererOpenGL::draw_line(DrawLineParams const& params)
	{
		//if (params.p1 == params.p2)
		//{
		//	return;
		//}

		//Quad area;

		//ASSERT(_activeRenderTarget != nullptr);
		//s32 w = _activeRenderTarget->width();
		//s32 h = _activeRenderTarget->height();

		//vec2 ar = { (h < w ? (real)h / (real)w : 1.0f), (w < h ? (real)w / (real)h : 1.0f) };

		//mat4 scale = glm::scale(mat4(1), vec3(ar.x, ar.y, 1.0f)); // TODO: remove this.

		//mat4 const& mvp = _activeRenderTarget->mvp();

		//vec3 p1 = glm::project(params.p1, mat4(1), mvp, vec4(-1, -1, 2, 2));
		//vec3 p2 = glm::project(params.p2, mat4(1), mvp, vec4(-1, -1, 2, 2));
		//
		//vec2 normal = normalize(vec2(p2) / ar - vec2(p1) / ar);
		//vec2 perpendicular = vec2(-normal.y, normal.x);
		//normal = scale * vec4(normal, 0, 1);
		//perpendicular = scale * vec4(perpendicular, 0, 1);

		//vec2 p1a = vec2(p1) + (-normal * params.width - perpendicular * params.width);
		//vec2 p1b = vec2(p1) + (-normal * params.width + perpendicular * params.width);

		//vec2 p2a;
		//vec2 p2b;

		//if (params.width2 != -1)
		//{
		//	p2a = vec2(p2) + (normal * params.width2 + perpendicular * params.width2);
		//	p2b = vec2(p2) + (normal * params.width2 - perpendicular * params.width2);
		//}
		//else
		//{
		//	p2a = vec2(p2) + (normal * params.width + perpendicular * params.width);
		//	p2b = vec2(p2) + (normal * params.width - perpendicular * params.width);
		//}

		//area.vertices[0].point = vec4(p1a, 0.0f, 1); // TODO: figure out depth of line
		//area.vertices[1].point = vec4(p1b, 0.0f, 1);
		//area.vertices[2].point = vec4(p2a, 0.0f, 1);
		//area.vertices[3].point = vec4(p2b, 0.0f, 1);

		Quad area = IRenderer::generate_line_quad(params, _activeRenderTarget);

		mat4 const& mvp = _activeRenderTarget->mvp(); // TODO: we are doing this twice
		vec3 p1 = glm::project(params.p1, mat4(1), mvp, vec4(-1, -1, 2, 2));
		vec3 p2 = glm::project(params.p2, mat4(1), mvp, vec4(-1, -1, 2, 2));

		struct ssbo_layout
		{
			vec3 p1;
			vec3 p2;
			Color c1;
			Color c2;
			real w1;
			real w2;
		};

		ssbo_layout layout;
		layout.p1 = p1;
		layout.p2 = p2;
		layout.c1 = params.color;
		layout.c2 = params.color2.a == -1 ? params.color : params.color2;
		layout.w1 = params.width;
		layout.w2 = params.width2 == -1 ? params.width : params.width2;

		ASSERT(_defaultLineMaterial != nullptr);
		ASSERT(_defaultLineMaterial->storage() == sizeof(layout)); // size of struct and ssbo do not match

		vector<shandle<ITexture>> empty_textures;

		auto f = _drawCalls.find(_activeRenderTarget);
		if (f == _drawCalls.end())
		{
			f = _drawCalls.insert({ _activeRenderTarget, {} });

		}

		auto& draw_call = (*f).second[_defaultLineMaterial][empty_textures];

		//auto& draw_call = _drawCalls[_activeRenderTarget][_defaultLineMaterial][empty_textures];
		u32 uniform_index = (u32)(draw_call.storage.size() / _defaultLineMaterial->storage()); // TODO: fix storage

		draw_call.vertices.push_back({ area.vertices[0], uniform_index }); // TODO: optimize this
		draw_call.vertices.push_back({ area.vertices[1], uniform_index });
		draw_call.vertices.push_back({ area.vertices[2], uniform_index });
		draw_call.vertices.push_back({ area.vertices[2], uniform_index });
		draw_call.vertices.push_back({ area.vertices[3], uniform_index });
		draw_call.vertices.push_back({ area.vertices[0], uniform_index });

		for (s32 i = 0; i < sizeof(layout); i++)
		{
			draw_call.storage.emplace_back(((u8*)(&layout))[i]);
		}
	}

	void RendererOpenGL::draw_text(Text const& text)
	{
		// TODO: this function is very incomplete
		s32 min_len = S32_MAX;
		s32 max_len = S32_MIN;

		string str = text.text;
		vector<pair<string, vec2>> lines;

		sstream stream(text.text);
		while (std::getline(stream, str))
		{
			min_len = MIN((s32)str.size(), min_len);
			max_len = MAX((s32)str.size(), max_len);
			lines.emplace_back(str, ORIGIN);
		}

		ASSERT(!lines.empty());

		vec3 scale;
		quat rotation;
		vec3 translation;
		vec3 skew;
		vec4 perspective;
		glm::decompose(text.transform, scale, rotation, translation, skew, perspective);

		for (s32 i = 0; i < lines.size(); i++)
		{
			string const& line = lines[i].first;
			vec2& origin = lines[i].second;

			switch (text.origin) // TODO: finish this
			{
			case ETextOrigin::TopLeft:
			{
				origin.x = scale.x;
				origin.y = -scale.y + -scale.y * i * 2;
				break;
			}
			case ETextOrigin::BottomCenter:
			{
				origin.x = -scale.x * line.size() + scale.x;
				origin.y = -scale.y + scale.y * (lines.size() - i) * 2;
				break;
			}
			}

			for (s32 j = 0; j < line.size(); j++)
			{
				auto& chr = line[j];

				ivec2 ascii_index;
				ascii_index.x = chr % 16;
				ascii_index.y = chr / 16;

				array<vec2, 4> texture_coords;
				texture_coords[0] = vec2(real(ascii_index.x) / 16.0f, 1.0f - real(ascii_index.y) / 16.0f);
				texture_coords[1] = vec2(real(ascii_index.x + 1) / 16.0f, 1.0f - real(ascii_index.y) / 16.0f);
				texture_coords[2] = vec2(real(ascii_index.x + 1) / 16.0f, 1.0f - real(ascii_index.y + 1) / 16.0f);
				texture_coords[3] = vec2(real(ascii_index.x) / 16.0f, 1.0f - real(ascii_index.y + 1) / 16.0f);

				reset_state(); // TODO: make it so we can use both materials and pass font as param
				activate_textures(text.font);
				draw_quad( // TODO: optimize
					QuadParams{
						.position = vec3((real)j * scale.x * 2, 0, 0) + translation + vec3(origin, 0.0f),
						.size = scale,
						.color = text.color,
						.texture_coords = texture_coords
					});

			}
		}
	}

	void RendererOpenGL::flush()
	{
		using iterator = map <
			shandle<ITexture>, // render targets
			map<
			shandle<MaterialOpenGL>, // materials
			map<
			vector<shandle<ITexture>>, // textures
			DrawCallOpenGL>
			>
		>::iterator;

		auto fn = [&](iterator i)
			{
				_vao.bind(); // TODO: may be able to move these outside the loop
				_vbo.bind();
				_ibo.bind();

				auto render_target = (*i).first; // TODO: add u8 overrideDepthBuffer to override the depth of the vertices and instead use the render targets stored depth

				// TODO: handle clearing render_target here

				ASSERT(render_target != nullptr);

				s32 w = render_target->width();
				s32 h = render_target->height();
				mat4 mvp = render_target->mvp();

				real wr = (h < w ? (real)h / (real)w : 1.0f);
				real hr = (w < h ? (real)w / (real)h : 1.0f);

				// render depth peels
				GLCall(glEnable(GL_DEPTH_TEST));
				GLCall(glDisable(GL_BLEND));

				// TODO: for all opaque materials, render to opaque peel, then for each depth peel, depth test against opaque peel when combining.
				// if opaque pixel is infront of the depth peel pixel: discard, else blend.
				for (s32 l = 0; l < _depthPeels.size(); l++)
				{
					_depthPeels[l]->bind_fbo();

					GLCall(glViewport(0, 0, w, h)); // TODO: may be able to move outer loop.
					GLCall(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
					GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

					// for each material.
					for (const auto& j : (*i).second)
					{
						auto& material = j.first; // TODO: add u8 to material _isFullyOpaque, if true, render to first depth peel only
						
						if (material->should_use_depth_testing)
						{
							GLCall(glEnable(GL_DEPTH_TEST));
						}
						else
						{
							GLCall(glDisable(GL_DEPTH_TEST));
						}

						if (material->should_use_blending)
						{
							GLCall(glEnable(GL_BLEND));
							GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // TODO: get blend func from material
						}
						else
						{
							GLCall(glDisable(GL_BLEND));
						}

						if (!material->should_use_depth_peeling)
						{
							if (l == 0)
							{
								continue;
							}

							// render directly to target and continue
							((handle<MaterialOpenGL>)(material))->bind(); // TODO: may be able to bind material at start of loop
							((handle<TextureOpenGL>)(render_target))->bind_fbo();

							// for each set of textures.
							for (const auto& k : j.second)
							{
								auto& textures = k.first;
								auto& draw_call = k.second;

								if (material->has_uniform("u_mvp"))
								{
									material->IMaterial::uniform("u_mvp", mvp);
								}

								if (material->has_uniform("u_viewport_size")) // TODO: update on on_window_resize
								{
									vec2 size;
									size.x = (real)w;
									size.y = (real)h;
									material->IMaterial::uniform("u_viewport_size", size);
								}

								if (material->has_uniform("u_has_texture"))
								{
									if (textures.empty())
									{
										material->IMaterial::uniform("u_has_texture", 0);
									}
									else
									{
										material->IMaterial::uniform("u_has_texture", 1);
									}
								}

								if (textures.size() == 1)
								{
									if (material->has_uniform("u_textures[0]"))
									{
										handle<ITexture> texture = textures[0];
										material->uniform("u_textures[0]", texture);
									}
									else if (material->has_uniform("u_texture"))
									{
										handle<ITexture> texture = textures[0];
										material->uniform("u_texture", texture);
									}
								}
								else
								{
									for (s32 o = 0; o < textures.size(); o++)
									{
										if (material->has_uniform("u_textures[" + to_string(o) + "]"))
										{
											handle<ITexture> texture = textures[o];
											material->uniform("u_textures[" + to_string(o) + "]", texture);
										}
									}
								}

								material->bind();
								GLCall(glViewport(0, 0, w, h));

								// TODO: render to depth peel

								_ssbo.data(draw_call.storage.data(), draw_call.storage.size());

								_vbo.data(draw_call.vertices.data(), (u32)draw_call.vertices.size());

								GLCall(glDrawElements(GL_TRIANGLES, _vbo.count(), GL_UNSIGNED_INT, nullptr));
							}

							continue;
						}

						_depthPeels[l]->bind_fbo();

						GLCall(glViewport(0, 0, w, h)); // TODO: may be able to move outer loop.
						//GLCall(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
						//GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

						//// handle depth testing and blending:
						//if (material->should_use_depth_testing || (material->should_use_depth_testing && material->should_use_depth_peeling))
						//{
						//	GLCall(glEnable(GL_DEPTH_TEST));
						//}
						//else
						//{
						//	GLCall(glDisable(GL_DEPTH_TEST));
						//}
						//
						//if (material->should_use_blending)
						//{
						//	GLCall(glEnable(GL_BLEND));
						//	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // TODO: get blend func from material
						//}
						//else
						//{
						//	GLCall(glDisable(GL_BLEND));
						//}
						//
						//// TODO: handle opaque materials.
						//if (l != 0 && !material->should_use_depth_peeling())
						//{
						//	continue;
						//}

						// for each set of textures.
						for (const auto& k : j.second)
						{
							auto& textures = k.first;
							auto& draw_call = k.second;

							if (material->has_uniform("u_mvp"))
							{
								material->IMaterial::uniform("u_mvp", mvp);
							}

							if (material->has_uniform("u_viewport_size")) // TODO: update on on_window_resize
							{
								vec2 size;
								size.x = (real)w;
								size.y = (real)h;
								material->IMaterial::uniform("u_viewport_size", size);
							}

							if (material->has_uniform("u_has_texture"))
							{
								if (textures.empty())
								{
									material->IMaterial::uniform("u_has_texture", 0);
								}
								else
								{
									material->IMaterial::uniform("u_has_texture", 1);
								}
							}

							if (textures.size() == 1)
							{
								if (material->has_uniform("u_textures[0]"))
								{
									handle<ITexture> texture = textures[0];
									material->uniform("u_textures[0]", texture);
								}
								else if (material->has_uniform("u_texture"))
								{
									handle<ITexture> texture = textures[0];
									material->uniform("u_texture", texture);
								}
							}
							else
							{
								for (s32 o = 0; o < textures.size(); o++)
								{
									if (material->has_uniform("u_textures[" + to_string(o) + "]"))
									{
										handle<ITexture> texture = textures[o];
										material->uniform("u_textures[" + to_string(o) + "]", texture);
									}
								}
							}

							if (material->has_uniform("u_depth_peel"))
							{
								material->IMaterial::uniform("u_depth_peel", (l == 0 ? 0 : 1));
							}

							material->bind();

							if (material->has_uniform("u_prev_depth") && l > 0)
							{
								handle<TextureOpenGL> const& prev = _depthPeels[l - 1];

								handle<ShaderOpenGL> shader = material->IMaterial::shader();
								s32 location = shader->uniformLocation("u_prev_depth");
								GLCall(glUniform1i(location, 31)); // TODO: test
								GLCall(glActiveTexture(GL_TEXTURE31));
								GLCall(glBindTexture(GL_TEXTURE_2D, prev->dbo())); // sneak in prev depth peel sampler2D.
							}

							_ssbo.data(draw_call.storage.data(), draw_call.storage.size());

							_vbo.data(draw_call.vertices.data(), (u32)draw_call.vertices.size());

							GLCall(glDrawElements(GL_TRIANGLES, _vbo.count(), GL_UNSIGNED_INT, nullptr));
						}
					}
				}

				// combine depth peels.
				vec2 ratio;
				handle<TextureOpenGL> rtgl = render_target;
				{
					ASSERT(rtgl->fbo() != 0);
					GLCall(glBindFramebuffer(GL_FRAMEBUFFER, rtgl->fbo()));
				}

				GLCall(glViewport(0, 0, w, h));

				GLCall(glDisable(GL_DEPTH_TEST));
				GLCall(glEnable(GL_BLEND));
				GLCall(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

				ratio = vec2(w, h) / vec2(RENDERER_DEPTH_PEEL_RESOLUTION);
				Quad quad(QuadParams{ .position = {0, 0, 0}, .size = {1, 1} });
				quad.vertices[0].texture_coord = vec2(0, 0);
				quad.vertices[1].texture_coord = vec2(ratio.x, 0);
				quad.vertices[2].texture_coord = { ratio.x, ratio.y };
				quad.vertices[3].texture_coord = vec2(0, ratio.y);

				VertexOpenGL vertices[6];
				vertices[0] = { quad.vertices[0], 0 };
				vertices[1] = { quad.vertices[1], 0 };
				vertices[2] = { quad.vertices[2], 0 };
				vertices[3] = { quad.vertices[2], 0 };
				vertices[4] = { quad.vertices[3], 0 };
				vertices[5] = { quad.vertices[0], 0 };

				for (s32 j = (s32)_depthPeels.size() - 1; j >= 0; j--)
				{
					_depthPeelMaterial->uniform("u_texture", _depthPeels[j]);
					_depthPeelMaterial->bind();
					_vbo.data(&vertices, 6);
					GLCall(glDrawElements(GL_TRIANGLES, _vbo.count(), GL_UNSIGNED_INT, nullptr));
				}
			};

		for (auto i = _drawCalls.begin(); i != _drawCalls.end(); i++)
		{
			fn(i);
		}

		_drawCalls.clear();
	}

	void RendererOpenGL::flush_render_graph(RenderGraph const& graph)
	{
		NIGHT_PROFILER_START_TIMER("Flush Render Graph To Renderer");
		auto& sorted_buffers = graph.sorted_buffers();

		reset_state();
		for (const auto& i : sorted_buffers) // render targets
		{
			auto& target = i.first;
			activate_render_target(target);

			for (const auto& j : i.second) // materials
			{
				auto material = handle<MaterialOpenGL>(j.first);

				if (material == nullptr)
				{
					material = _defaultMaterial;
				}

				activate_material(material);

				for (const auto& k : j.second) // textures
				{
					auto& textures = k.first;

					auto& graph = k.second;

					ASSERT(target != nullptr);

					auto f = _drawCalls.find(target);
					if (f == _drawCalls.end())
					{
						f = _drawCalls.insert({ target, {} });

					}

					auto& dc = (*f).second[material][textures];

					u32 current_uniform_index;
					if (material == nullptr || material->storage() == 0)
					{
						current_uniform_index = 0;
					}
					else
					{
						current_uniform_index = (u32)dc.storage.size() / material->storage();
					}
					
					size_t quads_vertices_size = graph.quads.size() * 6;
					size_t triangles_vertices_size = graph.triangles.size() * 3;
					vector<VertexOpenGL> vertices(quads_vertices_size + triangles_vertices_size);
					
					for (s32 i = 0; i < graph.quads.size(); i++)
					{
						Quad quad(graph.quads[i].first);
					
						vertices[i * 6].vertex = quad.vertices[0];
						vertices[i * 6 + 1].vertex = quad.vertices[1];
						vertices[i * 6 + 2].vertex = quad.vertices[2];
						vertices[i * 6 + 3].vertex = quad.vertices[2];
						vertices[i * 6 + 4].vertex = quad.vertices[3];
						vertices[i * 6 + 5].vertex = quad.vertices[0];
					
						vertices[i * 6].uniform_index = current_uniform_index + graph.quads[i].second;
						vertices[i * 6 + 1].uniform_index = current_uniform_index + graph.quads[i].second;
						vertices[i * 6 + 2].uniform_index = current_uniform_index + graph.quads[i].second;
						vertices[i * 6 + 3].uniform_index = current_uniform_index + graph.quads[i].second;
						vertices[i * 6 + 4].uniform_index = current_uniform_index + graph.quads[i].second;
						vertices[i * 6 + 5].uniform_index = current_uniform_index + graph.quads[i].second;
					}

					for (s32 i = 0; i < graph.triangles.size(); i++)
					{
						pair<Triangle, u32> const& triangle = graph.triangles[i];

						vertices[quads_vertices_size + i * 3].vertex = triangle.first.vertices[0];
						vertices[quads_vertices_size + i * 3 + 1].vertex = triangle.first.vertices[1];
						vertices[quads_vertices_size + i * 3 + 2].vertex = triangle.first.vertices[2];

						vertices[quads_vertices_size + i * 3].uniform_index = current_uniform_index + triangle.second;
						vertices[quads_vertices_size + i * 3 + 1].uniform_index = current_uniform_index + triangle.second;
						vertices[quads_vertices_size + i * 3 + 2].uniform_index = current_uniform_index + triangle.second;
					}
					
					dc.vertices.insert(dc.vertices.end(), vertices.begin(), vertices.end());
					dc.storage.insert(dc.storage.end(), graph.uniform_storage.begin(), graph.uniform_storage.end());

					for (const auto& l : graph.lines) // TODO: put lines directly into drawcall vertices
					{
						draw_line(l);
					}

					for (const auto& l : graph.texts) // TODO: put lines directly into drawcall vertices
					{
						draw_text(l);
					}
				}
			}
		}

		NIGHT_PROFILER_STOP_TIMER("Flush Render Graph To Renderer");
	}

	void RendererOpenGL::on_window_resize(u32 width, u32 height)
	{
		GLCall(glViewport(0, 0, width, height));

		ivec4 new_viewport;
		GLCall(glGetIntegerv(GL_VIEWPORT, (GLint*)&(new_viewport)));
		viewport(new_viewport);

		ASSERT(_defaultRenderTarget != nullptr);
		_defaultRenderTarget->resize(ivec2(width, height));
	}

	handle<IShader> RendererOpenGL::create_shader(const string& id, const ShaderParams& params)
	{
		auto i = find_shader(id);
		if (i != nullptr)
		{
			WARNING("shader already created!, id: ", id);
			return i;
		}

		shandle<IShader> shader(new ShaderOpenGL); // TODO: make opengl texture
		if (params.load_from_memory)
		{
			shader->load_from_memory(params.vs, params.fs);
		}
		else
		{
			shader->load(params.vs, params.fs);
		}

		shaders()[id] = (shandle<IShader>)shader;

		//TRACE("created shader, id: ", id);

		return shader;
	}

	handle<ITexture> RendererOpenGL::create_texture(const string& id, const TextureParams& params)
	{
		auto i = find_texture(id);
		if (i != nullptr)
		{
			WARNING("Texture already created!, id: ", id);
			return i;
		}

		shandle<TextureOpenGL> texture(new TextureOpenGL(params)); // TODO: make opengl texture
		textures()[id] = texture;

		//TRACE("created texture, id: ", id);

		return (handle<ITexture>)texture;
	}

	handle<IMaterial> RendererOpenGL::create_material(const string& id, const MaterialParams& params)
	{
		if (params.shader == nullptr)
		{
			WARNING("shader is nullptr, material not created.");
			return nullptr;
		}

		auto i = find_material(id);
		if (i != nullptr)
		{
			WARNING("material already created!, id: ", id);
			return i;
		}

		shandle<MaterialOpenGL> material(new MaterialOpenGL); // TODO: make opengl texture
		material->shader(params.shader);

		materials()[id] = material;

		//TRACE("created material, id: ", id);

		return (handle<IMaterial>)material;
	}

	handle<IComputeShader> RendererOpenGL::create_compute_shader(const string& id, const ComputeShaderParams& params)
	{
		auto i = find_compute_shader(id);
		if (i != nullptr)
		{
			WARNING("compute shader already created!, id: ", id);
			return i;
		}

		shandle<IComputeShader> shader(new ComputeShaderOpenGL); // TODO: make opengl texture
		if (params.load_from_memory)
		{
			shader->load_from_memory(params.path);
		}
		else
		{
			shader->load(params.path);
		}

		compute_shaders()[id] = (shandle<IComputeShader>)shader;

		//TRACE("created compute shader, id: ", id);

		return shader;
	}
}