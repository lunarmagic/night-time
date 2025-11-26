
#include "nightpch.h"
#include "Profiler.h"
#include "log/log.h"
#include "utility.h"

namespace night
{

	vector<string> Profiler::_timerStack;
	Profiler::Node Profiler::_root;
	Seconds Profiler::_historyLength = 1.0f;

#define NIGHT_PROFILER_TIMER_HISTORY_MAX 100

	void Profiler::push_timer(string const& name, Color color)
	{
		_timerStack.push_back(name);

		auto& node = find_timer(_timerStack);

		Timer timer;
		timer.start_point = std::chrono::high_resolution_clock::now();
		timer.timestamp = utility::window().time_elapsed();

		if (node.timer_history.empty())
		{
			timer.delta_time = _historyLength;
			node.timer_history.push_back(timer);
		}
		else if (timer.timestamp - node.timer_history.back().timestamp >= _historyLength)
		{
			timer.delta_time = _historyLength;
			node.timer_history.clear();
			node.timer_history.push_back(timer);
		}
		else
		{
			timer.delta_time = MIN(timer.timestamp - node.timer_history.back().timestamp, _historyLength);
			node.timer_history.push_back(timer);

			real time_acc = 0.0f;

			for (s32 i = 0; i < node.timer_history.size(); i++)
			{
				//time_acc += node.timer_history[i + 1].timestamp - node.timer_history[i].timestamp;
				time_acc += (real)node.timer_history[i].delta_time;
			}

			while (time_acc > _historyLength && node.timer_history.size() > 1)
			{
				Timer& front = node.timer_history.front();

				if (time_acc - front.delta_time > _historyLength)
				{
					time_acc -= (real)front.delta_time;
					node.timer_history.pop_front();
				}
				else
				{
					front.delta_time -= time_acc - _historyLength;
					break;
				}
			}
		}

		node.color = color;
	}

	void Profiler::pop_timer()
	{
		ASSERT(!_timerStack.empty()); // do not forget to pop.
		auto& node = find_timer(_timerStack);

		ASSERT(!node.timer_history.empty());
		auto& timer = node.timer_history.back();
		timer.end_point = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(timer.start_point).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(timer.end_point).time_since_epoch().count();

		timer.duration = (u64)(end - start);

		real sum = 0.0f;
		real count = 0;

		for (s32 i = 0; i < node.timer_history.size(); i++)
		{
			auto& timer = node.timer_history[i];
			sum += (real)(timer.duration * (timer.delta_time / _historyLength));
			//sum += timer.duration;
			//count += timer.delta_time;
			//count += 1;
		}

		//if (count > 0)
		//{
		//	sum /= count;
		//}

		node.average_time = sum;

		_timerStack.pop_back();
	}

	Profiler::Node& Profiler::find_timer(vector<string> const& path)
	{
		Node* current = &_root;

		for(s32 i = 0; i < path.size(); i++)
		{
			current = &current->children[path[i]];
		}

		return *current;
	}

}