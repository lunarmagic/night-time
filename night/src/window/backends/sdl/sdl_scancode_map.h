#pragma once

//#include "SDL3/SDL.h"
//#include "input/EInput.h"
//#include "stl/stl.h"

enum SDL_Scancode;
enum SDL_GamepadButton;

namespace night
{
	enum class EKey : u16;
	enum class EMouse : u8;
	enum class EButton : u8;

	extern map<SDL_Scancode, EKey> sdl_scancode_map;
	extern map<u8, EMouse> sdl_mouse_map;
	extern map<SDL_GamepadButton, EButton> sdl_controller_map;
}

