#pragma once

#include "core.h"
#include "color/Color.h"
#include "utility.h"

#define NIGHT_ENABLE_PROFILING

#ifdef NIGHT_ENABLE_PROFILING

//#define __NIGHT_PROFILER_PUSH(name, unique_str_name) { static const char unique_str_name[] = __FILE__; night::Profiler::push_timer(name, night::__ProfilerFileColor<unique_str_name>::color); }
//#define NIGHT_PROFILER_PUSH(name) __NIGHT_PROFILER_PUSH(name, UNIQUE_VARIABLE_NAME(unique_str_name))

#define NIGHT_PROFILER_POP() night::Profiler::pop_timer()

//#define __NIGHT_PROFILER_SCOPED(name, unique_str_name) static const char unique_str_name[] = __FILE__; night::ProfilerScoped UNIQUE_VARIABLE_NAME(profiler_scoped)(name, night::__ProfilerFileColor<unique_str_name>::color)
//#define NIGHT_PROFILER_SCOPED(name) __NIGHT_PROFILER_SCOPED(name, UNIQUE_VARIABLE_NAME(unique_str_name))

//#define NIGHT_PROFILER_SET_FILE_COLOR(color) template<> night::Color night::Profiler::__file_color<__FILE__>() { return color; }

#define NIGHT_PROFILER_SET_FILE_COLOR(c) template<> \
struct night::__ProfilerFileColor<__hash_string(__FILE__)> \
{ \
	inline static const Color color = c; \
};

#define NIGHT_PROFILER_PUSH(name) night::Profiler::push_timer(name, night::__ProfilerFileColor<__hash_string(__FILE__)>::color)
#define NIGHT_PROFILER_SCOPED(name) night::ProfilerScoped UNIQUE_VARIABLE_NAME(profiler_scoped)(name, night::__ProfilerFileColor<__hash_string(__FILE__)>::color)

#else
#define NIGHT_PROFILER_PUSH(name)
#define NIGHT_PROFILER_POP()
#define NIGHT_PROFILER_SCOPED(name)
#define NIGHT_PROFILER_SET_FILE_COLOR(color)
#endif

//template<char ...c>
//::night::Color __profiler_get_file_color();

namespace night
{

	template<size_t hash>
	struct __ProfilerFileColor
	{
		inline static const Color color = WHITE;
	};

	struct NIGHT_API Profiler
	{
		static void push_timer(string const& name, Color color);
		static void pop_timer();

		struct Timer
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> start_point;
			std::chrono::time_point<std::chrono::high_resolution_clock> end_point;
			u64 duration{ 0 };
			r64 timestamp{ 0.0f };
			r64 delta_time = 0.0f;
			Color color{ WHITE };
		};

		struct Node
		{
			deque<Timer> timer_history;
			map<string, Node> children;
			real average_time = -1.0f;
			Color color = LIGHT;
		};

		static vector<string> const& timer_stack() { return _timerStack; }
		static Node const& root() { return _root; }

		//template<char const* c>
		//static Color __file_color()
		//{
		//	return LIGHT;
		//}

	private:

		static Node& find_timer(vector<string> const& path);

		static vector<string> _timerStack;
		static Node _root;

		static Seconds _historyLength;

		friend struct DebugRenderer;
	};

	struct ProfilerScoped
	{
		ProfilerScoped(string const& name, Color color)
		{
			//NIGHT_PROFILER_PUSH(name);
			::night::Profiler::push_timer(name, color);
		}

		~ProfilerScoped()
		{
			NIGHT_PROFILER_POP();
		}
	};

}