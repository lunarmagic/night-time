
#include "nightpch.h"
#include "IShader.h"

namespace night
{
	set<handle<IShader>> IShader::_toBeInitialized;

	IShader::IShader(ShaderParams const& params, string const& id)
		: IResource(id, params.path.empty() ? "loaded from memory" : params.path)
	{
		_params = params;
	}

	void IShader::__tempInitHandle()
	{
		static mutex m;
		m.lock();
		_toBeInitialized.insert(handle_from_this());
		m.unlock();
	}

	void IShader::update_shaders()
	{
		for (auto& i : _toBeInitialized)
		{
			if (i != nullptr)
			{
				i->init();
			}
		}

		_toBeInitialized.clear();
	}
}