#pragma once

#include "IShader.h"
#include "math/math.h"
#include "resource_manager/IResource.h"

namespace night
{

	struct ComputeShaderParams
	{
		string path;
		string from_memory;
		//string path;
		//u8 load_from_memory{ false };
	};

	// do not dispatch compute shader in non-main thread
	struct NIGHT_API IComputeShader : public IResource
	{
		IComputeShader(ComputeShaderParams params, string const& id);
		virtual void init() = 0;
		virtual void __tempInitHandle() override;
		virtual void clean() = 0;

		void dispatch_to_main_thread(function<void(IComputeShader&)> fn, function<void(IComputeShader&)> callback);

		// TODO: implement reserve
		//virtual void reserve(s32 size, s32 binding = 0) = 0;

		virtual void data(void* data, size_t size, s32 binding = 0) = 0;

		// can return nullptr
		virtual void data(void* out_data, s32 binding = 0) = 0;

		// TODO: bind texture

		virtual void uniform1i(const string& name, s32 value) = 0;
		virtual void uniform1f(const string& name, real value) = 0;
		virtual void uniform2f(const string& name, real v0, real v1) = 0;
		virtual void uniform3f(const string& name, real v0, real v1, real v2) = 0;
		virtual void uniform4f(const string& name, real v0, real v1, real v2, real v3) = 0;
		virtual void uniformMat4f(const string& name, const mat4& matrix) = 0;

		virtual void compute() const = 0;

		ivec3 num_groups{ ivec3(1, 1, 1) };
		//void num_groups(ivec3 num) { _numGroups = num; }
		//ivec3 const& num_groups() const { return _numGroups; }

		static void update_compute_shaders();

	protected:

		ComputeShaderParams _params;
		virtual u8 load_from_path(const string& path) = 0;
		virtual u8 load_from_memory(const string& str) = 0;

	private:

		//void dispatch_to_main_thread_impl(function<void(IComputeShader&)> fn);

		struct MainThreadDispatch
		{
			function<void(IComputeShader&)> fn{ nullptr };
			function<void(IComputeShader&)> callback{ nullptr };
			handle<IComputeShader> compute_shader{ nullptr };

		};
		static vector<MainThreadDispatch> _toBeDispatchedInMainThread;

		static set<handle<IComputeShader>> _toBeInitialized;
		//ivec3 _numGroups{ ivec3(1, 1, 1) };
	};

}