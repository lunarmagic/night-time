
#include "nightpch.h"
#include "IComputeShader.h"
#include "log/log.h"

namespace night
{
	set<handle<IComputeShader>> IComputeShader::_toBeInitialized;
	vector<IComputeShader::MainThreadDispatch> IComputeShader::_toBeDispatchedInMainThread;

	IComputeShader::IComputeShader(ComputeShaderParams params, string const& id)
		: IResource(id, !params.path.empty() ? params.path : "loaded from memory")
	{
		_params = params;
	}

	void IComputeShader::__tempInitHandle()
	{
		static mutex m;
		m.lock();
		_toBeInitialized.insert(handle_from_this());
		m.unlock();
	}

	void IComputeShader::dispatch_to_main_thread(function<void(IComputeShader&)> fn, function<void(IComputeShader&)> callback)
	{
		ASSERT(fn != nullptr);
		static mutex m;
		m.lock();
		_toBeDispatchedInMainThread.push_back({ fn, callback, handle_from_this() });
		m.unlock();
	}

	void IComputeShader::update_compute_shaders()
	{
		for (auto& i : _toBeInitialized)
		{
			if (i != nullptr)
			{
				i->init();
			}
		}

		_toBeInitialized.clear();

		for (const auto& i : _toBeDispatchedInMainThread)
		{
			ASSERT(i.compute_shader != nullptr);
			ASSERT(i.fn != nullptr);

			i.fn(*i.compute_shader.ptr().lock());

			if (i.callback != nullptr)
			{
				i.callback(*i.compute_shader.ptr().lock());
			}
		}

		_toBeDispatchedInMainThread.clear();
	}

	//void IComputeShader::dispatch_to_main_thread_impl(function<void(IComputeShader&)> fn)
	//{
	//	ASSERT(_toBeDispatchedInMainThread == nullptr);
	//	_toBeDispatchedInMainThread = fn;
	//}
}