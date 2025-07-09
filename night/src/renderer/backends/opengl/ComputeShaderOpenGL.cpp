
#include "nightpch.h"
#include "ComputeShaderOpenGL.h"
#include "GLCall.h"
#include "log/log.h"

namespace night
{

	void ComputeShaderOpenGL::init()
	{
		if (!_params.path.empty())
		{
			load_from_path(_params.path);
		}
		else
		{
			load_from_memory(_params.from_memory);
		}
	}

    u8 ComputeShaderOpenGL::load_from_path(const string& path)
    {
        clean();

		ifstream file;
		file.open(path);

		if (file.is_open())
		{
			sstream buffer;
			buffer << file.rdbuf();

			string str = buffer.str();
			return load_from_memory(str);
		}

        return _rendererID != 0;
    }

    u8 ComputeShaderOpenGL::load_from_memory(const string& str)
    {
        clean();

		GLuint shaderHandle = glCreateShader(GL_COMPUTE_SHADER);

		const GLchar* source = (const GLchar*)str.c_str();
		GLCall(glShaderSource(shaderHandle, 1, &source, 0));

		GLCall(glCompileShader(shaderHandle));

		GLint isCompiled = 0;
		GLCall(glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &isCompiled));
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			GLCall(glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &maxLength));

			vector<GLchar> infoLog(maxLength);
			GLCall(glGetShaderInfoLog(shaderHandle, maxLength, &maxLength, &infoLog[0]));

			std::cerr << infoLog.data() << std::endl;

			GLCall(glDeleteShader(shaderHandle));
			return false;
		}

		GLCall(GLuint program = glCreateProgram());
		GLCall(glAttachShader(program, shaderHandle));
		GLCall(glLinkProgram(program));

		GLint isLinked = 0;
		GLCall(glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked));
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			GLCall(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength));

			vector<GLchar> infoLog(maxLength);
			GLCall(glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]));

			std::cerr << infoLog.data() << std::endl;

			GLCall(glDeleteProgram(program));
			GLCall(glDeleteShader(shaderHandle));

			return false;
		}

		GLCall(glDetachShader(program, shaderHandle));

		_rendererID = program;

        return _rendererID != 0;
    }

	//void ComputeShaderOpenGL::reserve(s32 size, s32 binding)
	//{
	//	auto f = _ssbos.find(binding);

	//	if (f == _ssbos.end())
	//	{
	//		ShaderStorageBufferOpenGL ssbo;
	//		ssbo.init(binding);
	//		_ssbos.insert({ binding, ssbo });
	//		f = _ssbos.find(binding);
	//	}

	//	ShaderStorageBufferOpenGL& ssbo = (*f).second;
	//	ssbo.reserve(size);
	//}

    void ComputeShaderOpenGL::data(void* data, size_t size, s32 binding)
    {
		
		auto f = _ssbos.find(binding);

		if (f == _ssbos.end())
		{
			ShaderStorageBufferOpenGL ssbo;
			ssbo.init(binding);
			_ssbos.insert({ binding, ssbo });
			f = _ssbos.find(binding);
		}

		ShaderStorageBufferOpenGL& ssbo = (*f).second;
		ssbo.data(data, size);
    }

	//void* ComputeShaderOpenGL::data(s32 binding)
	//{
	//	auto f = _ssbos.find(binding);
	//	if (f == _ssbos.end())
	//	{
	//		WARNING("compute shader ssbo binding not found");
	//		return nullptr;
	//	}

	//	return nullptr;
	//	//return (*f).second.data();
	//}

	void ComputeShaderOpenGL::data(void* out_data, s32 binding)
	{
		
		auto f = _ssbos.find(binding);
		if (f == _ssbos.end())
		{
			WARNING("compute shader ssbo binding not found");
			return;
		}

		(*f).second.data(out_data);
	}

	void ComputeShaderOpenGL::uniform1i(const string& name, s32 value)
	{
		
		// TODO: cache uniforms in map, bind once on compute.
		GLCall(glUseProgram(_rendererID));
		GLCall(glUniform1i(uniform_location(name), value));
	}

	void ComputeShaderOpenGL::uniform1f(const string& name, real value)
	{
		
		GLCall(glUseProgram(_rendererID));
		GLCall(glUniform1f(uniform_location(name), value));
	}

	void ComputeShaderOpenGL::uniform2f(const string& name, real v0, real v1)
	{
		
		GLCall(glUniform2f(uniform_location(name), v0, v1));
	}

	void ComputeShaderOpenGL::uniform3f(const string& name, real v0, real v1, real v2)
	{
		
		GLCall(glUseProgram(_rendererID));
		GLCall(glUniform3f(uniform_location(name), v0, v1, v2));
	}

	void ComputeShaderOpenGL::uniform4f(const string& name, real v0, real v1, real v2, real v3)
	{
		
		GLCall(glUseProgram(_rendererID));
		GLCall(glUniform4f(uniform_location(name), v0, v1, v2, v3));
	}

	void ComputeShaderOpenGL::uniformMat4f(const string& name, const mat4& matrix)
	{
		
		GLCall(glUseProgram(_rendererID));
		GLCall(glUniformMatrix4fv(uniform_location(name), 1, GL_FALSE, &matrix[0][0]));
	}

	void ComputeShaderOpenGL::compute() const
	{
		
		GLCall(glUseProgram(_rendererID));

		for (auto i = _ssbos.begin(); i != _ssbos.end(); i++)
		{
			(*i).second.bind();
		}

		GLCall(glDispatchCompute((GLuint)num_groups.x, (GLuint)num_groups.y, (GLuint)num_groups.z));
		GLCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
	}

	void ComputeShaderOpenGL::clean()
    {
		for (auto i = _ssbos.begin(); i != _ssbos.end(); i++)
		{
			(*i).second.clean();
		}

		if (_rendererID != 0)
		{
			GLCall(glDeleteProgram(_rendererID));
			TRACE("destroyed compute shader, id: ", _rendererID);
		}

		_rendererID = 0;
    }

	s32 ComputeShaderOpenGL::uniform_location(const string& name)
	{
		if (_uniformLocationCache.find(name) != _uniformLocationCache.end())
		{
			return _uniformLocationCache[name];
		}

		GLCall(int location = glGetUniformLocation(_rendererID, name.c_str()));

		if (location == -1)
		{
			WARNING("Warning: uniform ", name.c_str(), " doesn't exist!");
		}

		_uniformLocationCache[name] = location;
		return location;
	}

}

