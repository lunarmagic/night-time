#pragma warning(disable : 4251) // TODO: figure this out.
#pragma once

#ifdef NIGHT_DIST
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

//#include "Log.h"
//#include <SDL3/SDL_main.h>
//#undef _main

#ifdef NIGHT_PLATFORM_WINDOWS
extern night::Application* night::create_application();

#undef main // TODO: fix this

int main(int argc, char** argv)
{
	auto app = night::create_application();
	app->run();
	delete app;

	return 0;
}
#endif