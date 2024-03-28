#pragma once

namespace Input
{
	static int MouseX;
	static int MouseY;

	//void ParseKeyDown(SDL_Keycode Event);
	void ParseMouseMotion(int MouseX, int MouseY);
}