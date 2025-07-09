
#include "nightpch.h"
#include "ShaderOpenGL.h"
#include "GLCall.h"
#include "log/log.h"

namespace night
{
    void ShaderOpenGL::init()
    {
        if (_params.path.empty())
        {
            ASSERT(!_params.mem_vs.empty());
            ASSERT(!_params.mem_fs.empty());
            load_from_memory(_params.mem_vs, _params.mem_fs);
        }
        else
        {
            //load(_params.vs, _params.fs);
            load_from_path(_params.path);
        }
    }


    u8 ShaderOpenGL::load_from_path(string const& path)
    {
        ifstream file;
        file.open(path);
        if (file.is_open())
        {
            string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            string vdelimiter = "#Vertex\n";
            string fdelimiter = "#Fragment\n";

            size_t vend = data.find(vdelimiter);
            size_t fend = data.find(fdelimiter);
            ASSERT(vend != string::npos && fend != string::npos);

            string vertex = data.substr(vend + vdelimiter.size(), fend - vdelimiter.size());
            string fragment = data.substr(fend + fdelimiter.size(), data.size() - (fend + fdelimiter.size()));

            return load_from_memory(vertex, fragment);
        }
            
        ERROR("file is not valid");
        return false;

#if 0
        clean();

        auto load_file = [](const string& path) -> string
            {
                ifstream file;
                file.open(path);

                if (file.is_open())
                {
                    sstream buffer;
                    buffer << file.rdbuf();

                    return buffer.str();
                }

                return "";
            };

        string vs = load_file(path_vs);

        if (vs.empty())
        {
            ERROR("error loading vertex ShaderOpenGL! path: %s", path_vs.c_str());
            return false;
        }

        string fs = load_file(path_fs);

        if (fs.empty())
        {
            ERROR("error loading fragment ShaderOpenGL! path: %s", path_fs.c_str());
            return false;
        }

        _rendererID = createShader(vs, fs);

        _vsPath = path_vs;
        _fsPath = path_fs;

        return _rendererID != 0;
#endif
    }

    u8 ShaderOpenGL::load_from_memory(const string& vs, const string& fs)
    {
        clean();
        _rendererID = createShader(vs, fs);
        return _rendererID != 0;
    }

    void ShaderOpenGL::clean()
    {
        if (_rendererID != 0)
        {
            GLCall(glDeleteProgram(_rendererID));
            _rendererID = 0;
        }

        _vsPath.clear();
        _fsPath.clear();
        _uniformLocationCache.clear();
    }

    unsigned int ShaderOpenGL::compileShader(unsigned int type, const string& source)
    {
        GLCall(unsigned int id = glCreateShader(type));
        const char* src = source.c_str();
        GLCall(glShaderSource(id, 1, &src, nullptr));
        GLCall(glCompileShader(id));

        int result;
        GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
        if (result == GL_FALSE)
        {
            int length;
            GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* message = (char*)alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(id, length, &length, message));

            // TODO: use WARNING here
            //std::cout << "ERROR: Failed to compile " <<
            //    (type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<
            //    " shader!" << std::endl;
            //std::cout << message << std::endl;

            ERROR("ERROR: Failed to compile ", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), " shader!, message: ", message);

            GLCall(glDeleteShader(id));

            return 0;
        }

        return id;
    }

    unsigned int ShaderOpenGL::createShader(const string& vertexShader, const string& fragmentShader)
    {
        GLCall(unsigned int program = glCreateProgram());
        unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

        GLCall(glAttachShader(program, vs));
        GLCall(glAttachShader(program, fs));
        GLCall(glLinkProgram(program));
        GLCall(glValidateProgram(program));

        GLCall(glDeleteShader(vs));
        GLCall(glDeleteShader(fs));

        return program;
    }

    void ShaderOpenGL::bind() const
    {
        GLCall(glUseProgram(_rendererID));
    }

    void ShaderOpenGL::unbind() const
    {
        GLCall(glUseProgram(0));
    }

    void ShaderOpenGL::uniform1i(const string& name, s32 value)
    {
        GLCall(glUniform1i(uniformLocation(name), value));
    }

    void ShaderOpenGL::uniform1f(const string& name, real value)
    {
        GLCall(glUniform1f(uniformLocation(name), value));
    }

    void ShaderOpenGL::uniform2f(const string& name, real v0, real v1)
    {
        GLCall(glUniform2f(uniformLocation(name), v0, v1));
    }

    void ShaderOpenGL::uniform3f(const string& name, real v0, real v1, real v2)
    {
        GLCall(glUniform3f(uniformLocation(name), v0, v1, v2));
    }

    void ShaderOpenGL::uniform4f(const string& name, real v0, real v1, real v2, real v3)
    {
        GLCall(glUniform4f(uniformLocation(name), v0, v1, v2, v3));
    }

    void ShaderOpenGL::uniformMat4f(const string& name, const mat4& matrix)
    {
        GLCall(glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
    }

    int ShaderOpenGL::uniformLocation(const string& name)
    {
        if (_uniformLocationCache.find(name) != _uniformLocationCache.end())
        {
            return _uniformLocationCache[name];
        }
        
        GLCall(int location = glGetUniformLocation(_rendererID, name.c_str()));

        if (location == -1)
        {
            WARNING("Warning: uniform %s doesn't exist!", name.c_str());
            TRACE("vertex path: ", _vsPath.c_str());
            TRACE("fragment path: ", _fsPath.c_str());
            //return -1;
        }

        _uniformLocationCache[name] = location;
        return location;
    }

}

