
#include "nightpch.h"

//#include <windows.h>
#include "log.h"

#ifdef NIGHT_ENABLE_LOGGING
namespace night {

	//b8 Log::is_prev_cout_a_log = { false };
	s32 Log::debug_logging_mask = DEBUG_LOG_MASK_EVERYTHING;
	map<string, u32> Log::logged_messages = {};

	void Log::set_message_color(u16 id)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, id);
	}
}
#endif