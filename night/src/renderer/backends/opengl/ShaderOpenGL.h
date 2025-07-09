#pragma once

//#include <string>
//#include <unordered_map>

//#include "glm/glm.hpp"

#include "material/IShader.h"
#include "math/math.h"

namespace night
{

	struct ShaderOpenGL : public IShader
	{
		//ShaderOpenGL() = default;
		ShaderOpenGL(ShaderParams const& params, string const& id) : IShader(params, id) {}

		virtual void init() override;

		virtual u8 load_from_path(string const& path) override;
		virtual u8 load_from_memory(const string& vs, const string& fs) override;

		// TODO: use destructor
		virtual void clean() override;

		void bind() const;
		void unbind() const;

		virtual void uniform1i(const string& name, s32 value) override;
		virtual void uniform1f(const string& name, real value) override;
		virtual void uniform2f(const string& name, real v0, real v1) override;
		virtual void uniform3f(const string& name, real v0, real v1, real v2) override;
		virtual void uniform4f(const string& name, real v0, real v1, real v2, real v3) override;
		virtual void uniformMat4f(const string& name, const mat4& matrix);

		int uniformLocation(const string& name);

		uint32_t id() const { return _rendererID; }

	private:

		uint32_t _rendererID{ 0 };
		string _vsPath;
		string _fsPath;
		umap<string, int> _uniformLocationCache;

		unsigned int compileShader(unsigned int type, const string& source);
		unsigned int createShader(const string& vertexShader, const string& fragmentShader);
	};

}

