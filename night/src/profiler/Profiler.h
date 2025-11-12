#pragma once

#include "core.h"
#include "color/Color.h"
#include "utility.h"

#define NIGHT_ENABLE_PROFILING

#ifdef NIGHT_ENABLE_PROFILING
#define NIGHT_PROFILER_PUSH(name) ::night::Profiler::push_timer(name)
#define NIGHT_PROFILER_POP() ::night::Profiler::pop_timer()
#define NIGHT_PROFILER_SCOPED(name) ::night::ProfilerScoped UNIQUE_VARIABLE_NAME(profiler_scoped)(name)

#else
#define NIGHT_PROFILER_PUSH(name)
#define NIGHT_PROFILER_POP()
#define NIGHT_PROFILER_SCOPED(name)
#endif

namespace night
{

	struct NIGHT_API Profiler
	{
		static void push_timer(string const& name);
		static void pop_timer();

		struct Timer
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> start_point;
			std::chrono::time_point<std::chrono::high_resolution_clock> end_point;
			u64 duration{ 0 };
			r64 time_stamp{ 0.0f };
			Color color{ WHITE };
		};

		struct Node
		{
			deque<Timer> timer_history;
			map<string, Node> children;
		};

		static vector<string> const& timer_stack() { return _timerStack; }
		static Node const& root() { return _root; }

	private:

		static Node& find_timer(vector<string> const& path);

		static vector<string> _timerStack;
		static Node _root;

		friend struct DebugRenderer;
	};

	struct ProfilerScoped
	{
		ProfilerScoped(string const& name)
		{
			NIGHT_PROFILER_PUSH(name);
		}

		~ProfilerScoped()
		{
			NIGHT_PROFILER_POP();
		}
	};

}