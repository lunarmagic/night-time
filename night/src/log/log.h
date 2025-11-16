#pragma once

#include "core.h"
//#include "stl/stl.h"
//#include <iostream>

#undef LOG
#undef WARNING
#undef ERROR

#ifdef NIGHT_ENABLE_LOGGING
#define PRINT(x) ::night::Log::print(x, #x)
#define TRACE(str, ...) ::night::Log::trace(str, ##__VA_ARGS__)
#define WARNING(str, ...) ::night::Log::warning(__FILE__, __LINE__, str, ##__VA_ARGS__)
#define ERROR(str, ...) ::night::Log::error(__FILE__, __LINE__, str, ##__VA_ARGS__); __debugbreak();
#else
#define PRINT(x)
#define TRACE(str, ...)
#define WARNING(str, ...)
#define ERROR(str, ...)
#endif


#ifdef NIGHT_ENABLE_LOGGING
#define LOG_MAX_MESSAGES 5

#define DEBUG_LOG_MASK_OFF 0
#define DEBUG_LOG_MASK_PRINT BIT(1)
#define DEBUG_LOG_MASK_ERROR BIT(2)
#define DEBUG_LOG_MASK_WARNING BIT(3)
#define DEBUG_LOG_MASK_TRACE BIT(4)
#define DEBUG_LOG_MASK_EVERYTHING S32_MAX

namespace night
{

	struct NIGHT_API Log
	{
		//static u8 is_prev_cout_a_log;
		static map<string, u32> logged_messages;

		static s32 debug_logging_mask;

		static void set_message_color(u16 id);

		template<typename T>
		static string print_format(T& t)
		{
			return to_string(t);
		}

		template<typename T>
		static void log(vector<string>& out_vector, T&& t)
		{
			out_vector.push_back(print_format(t));
		}

		template<typename T, typename... Args>
		static void log(vector<string>& out_vector, T&& t, Args&&... args)
		{
			out_vector.push_back(print_format(t));
			log(out_vector, std::forward<Args&&>(args)...);
		}

		template<typename... Args>
		static void log(string const& str, Args&&... args)
		{
			//if (!is_prev_cout_a_log)
			//{
			//	std::cout << '\n';
			//}

			vector<string> arg_strings;
			log(arg_strings, std::forward<Args&&>(args)...);

			s32 curr = 0;

			for (s32 i = 0; i < str.size();)
			{
				if (i < str.size() - 2)
				{
					char const& c = str[i];
					char const& c2 = str[i + 2];
					s32 index = s32(str[i + 1] - '0');

					if (index < arg_strings.size() && c == '{' && c2 == '}')
					{
						std::cout << str.substr(curr, i - curr);
						std::cout << arg_strings[index];
						i += 3;
						curr = i;
						continue;
					}
				}

				i++;
			}

			std::cout << str.substr(curr, str.size() - curr);

			//is_prev_cout_a_log = true;
		}

		template<typename... Args>
		static void log(string const& str)
		{
			//if (!is_prev_cout_a_log)
			//{
			//	std::cout << '\n';
			//}

			std::cout << str;

			//is_prev_cout_a_log = true;
		}

		template<typename... Args>
		static void message(const char* type, const char* file, s32 line, string const& str, Args&&... args)
		{
			sstream stream;
			string key = string(type) + string(file) + to_string(line);
			
			auto i = logged_messages.find(key);
			if (i != logged_messages.end())
			{
				if ((*i).second >= LOG_MAX_MESSAGES)
				{
					return;
				}
			
				(*i).second++;
				stream << "(" << (*i).second << ") ";
			}
			else
			{
				logged_messages.insert({ key, 1 });
			}
			
			stream << type << "\n  File: " << file << ",\n  Line: " << line << ",\n  Message: ";
			std::cout << '\n' << stream.str();
			log(str, std::forward<Args&&>(args)...);
			std::cout << '\n';
		}

		template<typename... Args>
		static void trace(string const& str, Args&&... args)
		{
			if (!(debug_logging_mask & DEBUG_LOG_MASK_TRACE))
			{
				return;
			}

			set_message_color(7);

			log(str, args...);
			log("\n");
		}

		template<typename... Args>
		static void trace(string const& str)
		{
			if (!(debug_logging_mask & DEBUG_LOG_MASK_TRACE))
			{
				return;
			}

			set_message_color(7);

			log(str);
			log("\n");
		}

		template<typename T>
		static void print(T& t, const char* var_name)
		{
			if (!(debug_logging_mask & DEBUG_LOG_MASK_PRINT))
			{
				return;
			}

			set_message_color(11);
			
			//log(typeid(T).name(), " ", var_name, ": ");
			std::cout << typeid(T).name() << " " << var_name << ": ";
			log("\n");
			log(print_format(t));
			log("\n");
		}

		template<typename... Args>
		static void warning(const char* file, s32 line, string const& str, Args&&... args)
		{
			if (!(debug_logging_mask & DEBUG_LOG_MASK_WARNING))
			{
				return;
			}
			
			set_message_color(14);
			message("Warning!", file, line, str, std::forward<Args&&>(args)...);
		}

		template<typename... Args>
		static void error(const char* file, s32 line, string const& str, Args&&... args)
		{
			if (!(debug_logging_mask & DEBUG_LOG_MASK_ERROR))
			{
				return;
			}
			
			set_message_color(12);
			message("Error!", file, line, str, std::forward<Args&&>(args)...);
		}
	};
}
#endif
