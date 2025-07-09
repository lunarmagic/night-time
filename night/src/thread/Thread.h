#pragma once

//#include "stl/stl.h"

namespace night
{
	template<typename Return, typename... Args>
	struct Thread
	{
		Thread() = default;
		Thread(function<Return(Args&&...)> fn, Args&&... args);

		u8 valid() const { return _future.valid(); }

		void wait();

	private:

		future<Return> _future;
	};
	
	template<typename Return, typename ...Args>
	inline Thread<Return, Args...>::Thread(function<Return(Args&&...)> fn, Args&&... args)
	{
#if defined(NIGHT_DISABLE_MULTI_THREADING) || defined(NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER)
		fn(args...);
#else
		_future = async(fn, args...);
#endif
	}

	template<typename Return, typename ...Args>
	inline void Thread<Return, Args...>::wait()
	{
		if (_future.valid())
		{
			_future.wait();
		}
	}
}