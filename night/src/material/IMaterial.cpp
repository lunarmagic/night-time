
#include "nightpch.h"
#include "IMaterial.h"

namespace night
{
	set<handle<IShader>> IMaterial::_toBeInitialized;

	IMaterial::IMaterial(MaterialParams const& params, string const& id)
		: IResource(id, params.shader->path())
	{
		_shader = params.shader;
	}

	void IMaterial::__tempInitHandle()
	{
		static mutex m;
		m.lock();
		_toBeInitialized.insert(handle_from_this());
		m.unlock();
	}

	u8 IMaterial::should_use_depth_peeling() const
	{
		return (has_uniform("u_prev_depth") && has_uniform("u_depth_peel"));
	}

	void IMaterial::update_materials()
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