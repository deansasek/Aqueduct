#include "../Common.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/rotate_vector.hpp"

Engine::Camera::CameraStruct Engine::Camera::Camera;
glm::vec2 Engine::Camera::OldMousePosition;

void Engine::Camera::CreateCamera()
{
	Engine::Camera::CameraStruct Camera{};
	Camera.Eye = glm::vec3(0.0f, 0.0f, 0.0f);
	Camera.ViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	Camera.UpVector = glm::vec3(0.0f, 1.0f, 0.0f);

	Engine::Camera::Camera = Camera;

	std::cout << "CAMERA > Camera successfully created! \n";
}

glm::mat4 Engine::Camera::GetViewMatrix()
{
	return glm::lookAt(Engine::Camera::Camera.Eye, Engine::Camera::Camera.Eye + Engine::Camera::Camera.ViewDirection, Engine::Camera::Camera.UpVector);
}

void Engine::Camera::MoveUp(float Speed)
{
	Engine::Camera::Camera.Eye.y += Speed;
}

void Engine::Camera::MoveDown(float Speed)
{
	Engine::Camera::Camera.Eye.y -= Speed;
}

void Engine::Camera::MoveForward(float Speed)
{
	Engine::Camera::Camera.Eye += (Engine::Camera::Camera.ViewDirection * Speed);
}

void Engine::Camera::MoveBackward(float Speed)
{
	Engine::Camera::Camera.Eye -= (Engine::Camera::Camera.ViewDirection * Speed);
}

void Engine::Camera::MoveLeft(float Speed)
{
	glm::vec3 RightVector = glm::cross(Engine::Camera::Camera.ViewDirection, Engine::Camera::Camera.UpVector);
	Engine::Camera::Camera.Eye -= RightVector * Speed;
}

void Engine::Camera::MoveRight(float Speed)
{
	glm::vec3 RightVector = glm::cross(Engine::Camera::Camera.ViewDirection, Engine::Camera::Camera.UpVector);
	Engine::Camera::Camera.Eye += RightVector * Speed;

}

void Engine::Camera::MouseLook(int MouseX, int MouseY)
{
	glm::vec2 CurrentMouse = glm::vec2(MouseX, MouseY);

	static bool FirstLook = true;
	if (FirstLook)
	{
		Engine::Camera::OldMousePosition = CurrentMouse;
		FirstLook = false;
	}

	glm::vec2 MouseDelta = Engine::Camera::OldMousePosition - CurrentMouse;

	/*
		Find a way to make this more efficient
	*/
	

	// Left and right
	Camera::Camera.ViewDirection = glm::rotate(Engine::Camera::Camera.ViewDirection, glm::radians(MouseDelta.x), Engine::Camera::Camera.UpVector);
	// Up and down
	Camera::Camera.ViewDirection = glm::rotate(Engine::Camera::Camera.ViewDirection, glm::radians(MouseDelta.y), glm::cross(Engine::Camera::Camera.ViewDirection, Engine::Camera::Camera.UpVector));

	Camera::OldMousePosition = CurrentMouse;
}