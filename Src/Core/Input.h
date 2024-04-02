#pragma once

namespace Engine
{
	namespace Input
	{
		static int MouseX;
		static int MouseY;

		void ParseEvent();
		void ParseMouseMotion(int MouseX, int MouseY);
	}
}