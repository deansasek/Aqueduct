#include "Input.h"
#include "../Common.h"

#include "Camera.h"

//void Input::ParseKeyDown(SDL_Keycode Event)
//{
//	std::cout << Event;
//}

void Input::ParseMouseMotion(int MouseX, int MouseY)
{
	Input::MouseX += MouseX;
	Input::MouseY += MouseY;

	Camera::MouseLook(Input::MouseX, Input::MouseY);
}