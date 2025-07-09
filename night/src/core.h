#pragma warning(disable : 4251) // TODO: figure this out.
#pragma once

//#include "types/types.h"

#ifdef NIGHT_PLATFORM_WINDOWS
	#ifdef NIGHT_BUILD_DLL
		#define NIGHT_API __declspec(dllexport)
	#else
		#define NIGHT_API __declspec(dllimport)
	#endif
#else
	#error Light only supports Windows!
#endif