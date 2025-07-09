
#include "nightpch.h"
#include "Profiler.h"
#include "log/log.h"
#include "utility.h"

// TODO: hide behind debug macro

namespace night
{

	//map<string, Profiler::Timer> Profiler::_timers;

	vector<string> Profiler::_timerStack;
	Profiler::Node Profiler::_root;

#define NIGHT_PROFILER_TIMER_HISTORY_MAX 100

	void Profiler::push_timer(string const& name)
	{
		_timerStack.push_back(name);

		auto& node = find_timer(_timerStack);

		if (node.timer_history.size() > NIGHT_PROFILER_TIMER_HISTORY_MAX)
		{
			node.timer_history.pop_front();
		}

		auto& timer = node.timer_history.emplace_back();
		timer.start_point = std::chrono::high_resolution_clock::now();



		//_timers[name].start_point = std::chrono::high_resolution_clock::now();
	}

	void Profiler::pop_timer()
	{
		ASSERT(!_timerStack.empty()); // do not forget to pop.
		auto& node = find_timer(_timerStack);

		ASSERT(!node.timer_history.empty());
		auto& timer = node.timer_history.back();
		//node.timer.end_point = std::chrono::high_resolution_clock::now();
		timer.end_point = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(timer.start_point).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(timer.end_point).time_since_epoch().count();

		timer.duration = (u64)(end - start);
		//timer.time_stamp = (r64)(start) / 1000000.0f;
		timer.time_stamp = utility::window().time_elapsed();

		_timerStack.pop_back();
		//ASSERT(_timers.find(name) != _timers.end()); // forgot to start timer.
		//auto& timer = _timers[name];
		//timer.end_point = std::chrono::high_resolution_clock::now();
		//
		//auto start = std::chrono::time_point_cast<std::chrono::microseconds>(timer.start_point).time_since_epoch().count();
		//auto end = std::chrono::time_point_cast<std::chrono::microseconds>(timer.end_point).time_since_epoch().count();

		//timer.duration = (u64)(end - start);
	}

	Profiler::Node& Profiler::find_timer(vector<string> const& path)
	{
		Node* current = &_root;

		for(s32 i = 0; i < path.size(); i++)
		{
			current = &current->children[path[i]];
		}

		return *current;

		//ASSERT(_timers.find(name) != _timers.end()); // forgot to start timer.
		//ASSERT(_timers[name].duration != 0); // forgot to end timer.

		//return _timers[name].duration;
	}

}