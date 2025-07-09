#pragma once

#include "core.h"
#include "color/Color.h"
//#include "stl/stl.h"
//#include <chrono>

#define NIGHT_ENABLE_PROFILING

// TODO: handle threads
// TODO: add color coding based on object

//inline std::string class_name(const std::string& prettyFunction)
//{
//	size_t colons = prettyFunction.find("::");
//	if (colons == std::string::npos)
//		return "::";
//	size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
//	size_t end = colons - begin;
//
//	return prettyFunction.substr(begin, end);
//}

//#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)

#ifdef NIGHT_ENABLE_PROFILING
#define NIGHT_PROFILER_PUSH(name) ::night::Profiler::push_timer(name)
#define NIGHT_PROFILER_POP() ::night::Profiler::pop_timer()
#define NIGHT_PROFILER_SCOPED(name) ::night::ProfilerScoped name(#name)
// TODO: add scoped timer
//#define NIGHT_PROFILER_TIMER_DURATION(name) ::night::Profiler::timer_duration(name)
//#define NIGHT_PROFILER_PRINT_TIMER(name) TRACE("timer ", #name, ": ", ::night::Profiler::timer_duration(name))
#else
#define NIGHT_PROFILER_PUSH(name)
#define NIGHT_PROFILER_POP()
#define NIGHT_PROFILER_SCOPED(name)
//#define NIGHT_PROFILER_TIMER_DURATION(name)
//#define NIGHT_PROFILER_PRINT_TIMER(name)
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

		//static map<string, Timer> const& timers() { return _timers; }
		static vector<string> const& timer_stack() { return _timerStack; }
		static Node const& root() { return _root; }

	private:

		static Node& find_timer(vector<string> const& path);

		//static map<string, Timer> _timers;
		static vector<string> _timerStack;
		static Node _root;
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