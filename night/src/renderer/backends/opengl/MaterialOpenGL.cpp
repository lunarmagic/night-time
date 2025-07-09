
#include "nightpch.h"
#include "MaterialOpenGL.h"
#include "ShaderOpenGL.h"
#include "GLCall.h"
#include "math/math.h"
#include "TextureOpenGL.h"
#include <GL/glew.h>
#include "RendererOpenGL.h"

namespace night
{
	void MaterialOpenGL::init()
	{
		ASSERT(_shader != nullptr);

		query_uniforms();
		query_storage();
	}

	void MaterialOpenGL::clean()
	{
		clear_uniforms();
	}

#if 0
	// TODO: the texture should not be destroyed untill all materials unbind it.
	u8 MaterialOpenGL::uniform(const string& name, handle<ITexture> texture)
	{
		ASSERT(texture != nullptr);

		auto i = _uniforms.find(name);
		if (i != _uniforms.end())
		{
			auto& uniform = (*i).second;

			// TODO: check type and size.

			if (uniform.index + sizeof(TextureUniformData) > _data.size())
			{
				WARNING("data out of bounds, name: %s", name.c_str());
				return false;
			}

			(TextureUniformData&)_data[uniform.index] = TextureUniformData{ .texture = texture, .sample_depth_buffer = false };

			return true;
		}

		WARNING("uniform not found!, name: %s", name.c_str());
		return false;
	}

	u8 MaterialOpenGL::uniform(string const& name, DepthBuffer depth_buffer)
	{
		ASSERT(depth_buffer != nullptr);

		auto i = _uniforms.find(name);
		if (i != _uniforms.end())
		{
			auto& uniform = (*i).second;

			// TODO: check type and size.

			if (uniform.index + sizeof(TextureUniformData) > _data.size())
			{
				WARNING("data out of bounds, name: %s", name.c_str());
				return false;
			}

			(TextureUniformData&)_data[uniform.index] = TextureUniformData{ .texture = depth_buffer, .sample_depth_buffer = true };

			return true;
		}

		WARNING("uniform not found!, name: %s", name.c_str());
		return false;
	}
#endif

	u8 MaterialOpenGL::uniform(const string& name, TextureUniformData const& texture)
	{
		ASSERT(texture.texture != nullptr);

		auto i = _uniforms.find(name);
		if (i != _uniforms.end())
		{
			auto& uniform = (*i).second;

			// TODO: check type and size.

			if (uniform.index + sizeof(TextureUniformData) > _data.size())
			{
				WARNING("data out of bounds, name: %s", name.c_str());
				return false;
			}

			(TextureUniformData&)_data[uniform.index] = texture;

			return true;
		}

		WARNING("uniform not found!, name: %s", name.c_str());
		return false;
	}

	void MaterialOpenGL::query_uniforms()
	{
		ref<ShaderOpenGL> shader = ref<ShaderOpenGL>(_shader);
		u32 id = shader->id();

		clear_uniforms();
		_data.clear();

		GLint count;
		GLsizei length;
		const GLsizei buffer_size = 16;

		GLint size;
		GLenum type;
		GLchar buffer[buffer_size];

		GLCall(glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count));

		uint32_t index = 0;

		for (GLint j = 0; j < count; j++)
		{
			GLCall(glGetActiveUniform(id, (GLuint)j, buffer_size, &length, &size, &type, buffer));

			//// TODO: remove u_mvp from uniforms, set it manually by renderer
			//if (
			//	!strcmp(buffer, "u_atlas[0]") || \
			//	!strcmp(buffer, "u_mvp") || \
			//	!strcmp(buffer, "u_prevDepth") || \
			//	!strcmp(buffer, "u_depthPeel"))
			//{
			//	continue;
			//}

			uint32_t temp = index;

			switch (type)
			{
			case GL_INT:
				temp += size * sizeof(GLint);
				break;

			case GL_FLOAT:
				temp += size * sizeof(GLfloat);
				break;

			case GL_FLOAT_VEC2:
				temp += size * sizeof(vec2);
				break;

			case GL_FLOAT_VEC3:
				temp += size * sizeof(vec3);
				break;

			case GL_FLOAT_VEC4:
				temp += size * sizeof(vec4);
				break;

			case GL_FLOAT_MAT4:
				temp += size * sizeof(mat4);
				break;

			case GL_SAMPLER_2D:
				//temp += size * sizeof(GLuint);
				//temp += size * sizeof(shandle<TextureOpenGL>);
				temp += size * sizeof(TextureUniformData);
				break;

			default:
				WARNING("uniform type not supported, name: %s, type: %d", buffer, type);
				continue; // skip this uniform if not supported.
			}

			if (size > 1)
			{
				for (s32 i = 0; i < size; i++)
				{
					//buffer[length - 1] = '0' + i;
					buffer[length - 2] = '\0';
					string name(buffer + to_string(i) + ']');
					u32 j = index + sizeof(GLuint) * i;
					_uniforms[name] = { .type = type, .size = size / size, .index = j };
					TRACE("uniform name: ", name, ", type: ", type, ", size: ", size);
				}
			}
			else
			{
				string name(buffer);
				_uniforms[name] = { .type = type, .size = size, .index = index };
				TRACE("uniform name: ", name, ", type: ", type, ", size: ", size);
			}

			index = temp;
		}

		_data = vector<uint8_t>(index, 0x00);

		// TODO: may need to manually set textures to nullptr.

		TRACE("uniform cpu buffer size in bytes: ", _data.size());
		TRACE("");
		//LOG("uniform cpu buffer size in bytes: %d", _data.size());
		//LOG("\n");
	}

	void MaterialOpenGL::query_storage()
	{
		ref<ShaderOpenGL> shader = ref<ShaderOpenGL>(_shader);
		_storage = 0;

		GLuint id = shader->id();
		GLint num_resources;
		GLint ssbo_max_len;
		GLint var_max_len;

		GLCall(glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &num_resources));
		GLCall(glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len));
		GLCall(glGetProgramInterfaceiv(id, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len));

		string name;

		for (auto i = 0; i < num_resources; i++)
		{
			GLsizei str_length;
			GLCall(glGetProgramResourceName(id, GL_SHADER_STORAGE_BLOCK, i, ssbo_max_len, &str_length, name.data()));

			GLint ssbo_index;
			GLCall(ssbo_index = glGetProgramResourceIndex(id, GL_SHADER_STORAGE_BLOCK, name.data()));

			GLint binding;
			GLenum prop = GL_BUFFER_BINDING;
			glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, ssbo_index, 1, &prop, 1, nullptr, &binding);

			if (binding != NIGHT_RENDERER_OPENGL_UNIFORM_BUFFER_BINDING)
			{
				continue;
			}

			GLint arr_index;
			prop = GL_ACTIVE_VARIABLES;
			glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, ssbo_index, 1, &prop, 1, nullptr, &arr_index);

			prop = GL_TOP_LEVEL_ARRAY_STRIDE;
			GLint stride;
			GLCall(glGetProgramResourceiv(id, GL_BUFFER_VARIABLE, arr_index, 1, &prop, 1, nullptr, &stride));

			if (stride > 0)
			{
				_storage = stride;
				TRACE("uniform buffer added, name: ", name.data(), ", stride:",  stride);
				return;
			}
		}
	}

	void MaterialOpenGL::clear_uniforms()
	{
		for (auto i = _uniforms.begin(); i != _uniforms.end(); i++)
		{
			auto& uniform = (*i).second;
			if (uniform.type == GL_SAMPLER_2D)
			{
				ASSERT(uniform.index < _data.size() && uniform.index >= 0);
				((TextureUniformData&)_data[uniform.index]).texture = nullptr; // decrement ref count
			}
		}

		_uniforms.clear();
	}

	u8 MaterialOpenGL::bind()
	{
		ref<ShaderOpenGL> shader = ref<ShaderOpenGL>(_shader);

		if (shader != nullptr)
		{
			shader->bind();

			int texture_count = 0;
			for (auto& i : _uniforms)
			{
				const string& name = i.first;
				int location = shader->uniformLocation(name);
				const int& index = i.second.index;
				const int& size = i.second.size;

				switch (i.second.type)
				{
				case GL_INT:
				{
					const GLint* value = (GLint*)&_data[index];
					GLCall(glUniform1iv(location, size, value));
				} break;

				case GL_FLOAT:
				{
					const GLfloat* value = (float*)&_data[index];
					GLCall(glUniform1fv(location, size, value));
				} break;

				case GL_FLOAT_VEC2:
				{
					const float* value = (float*)&_data[index];
					GLCall(glUniform2fv(location, size, value));
				} break;

				case GL_FLOAT_VEC3:
				{
					const float* value = (float*)&_data[index];
					GLCall(glUniform3fv(location, size, value));
				} break;

				case GL_FLOAT_VEC4:
				{
					const float* value = (float*)&_data[index];
					GLCall(glUniform4fv(location, size, value));
				} break;

				case GL_FLOAT_MAT4:
				{
					const float* value = (float*)&_data[index];
					GLCall(glUniformMatrix4fv(location, size, GL_FALSE, value));
				} break;

				case GL_SAMPLER_2D:
				{
					int location = shader->uniformLocation(i.first);
					//const GLuint* value = ((GLuint*)&_data[i.second.index]);
					TextureUniformData const& value = ((TextureUniformData const&)_data[i.second.index]);
					handle<const TextureOpenGL> const& texture = (handle<const TextureOpenGL> const&)value.texture;
					if (texture != nullptr)
					{
						GLCall(glUniform1i(location, texture_count)); // TODO: test
						GLCall(glActiveTexture(GL_TEXTURE0 + texture_count));
						if (!value.sample_depth_buffer)
						{
							GLCall(glBindTexture(GL_TEXTURE_2D, texture->id()));
						}
						else
						{
							//GLCall(glUniform1i(location, 31)); // TODO: test
							//GLCall(glActiveTexture(GL_TEXTURE31));
							//GLCall(glBindTexture(GL_TEXTURE_2D, prev->dbo())); // sneak in prev depth peel sampler2D.
							GLCall(glBindTexture(GL_TEXTURE_2D, texture->dbo()));
						}
					}
					else
					{
						GLCall(glUniform1i(location, texture_count)); // TODO: test
						GLCall(glActiveTexture(GL_TEXTURE0 + texture_count));
						GLCall(glBindTexture(GL_TEXTURE_2D, 0));
					}

					texture_count++;
				} break;
				}
			}

			return true;
		}

		return false;
	}
}