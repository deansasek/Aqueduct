#include "../Common.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/rotate_vector.hpp"

Camera::CameraStruct Camera::Camera;
glm::vec2 Camera::OldMousePosition;

void Camera::CreateCamera()
{
	Camera::CameraStruct Camera{};
	Camera.Eye = glm::vec3(0.0f, 0.0f, 0.0f);
	Camera.ViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	Camera.UpVector = glm::vec3(0.0f, 1.0f, 0.0f);

	Camera::Camera = Camera;

	std::cout << "CAMERA > Camera successfully created! \n";
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(Camera::Camera.Eye, Camera::Camera.Eye + Camera::Camera.ViewDirection, Camera::Camera.UpVector);
}

void Camera::MoveForward(float Speed)
{
	Camera::Camera.Eye += (Camera::Camera.ViewDirection * Speed);
}

void Camera::MoveBackward(float Speed)
{
	Camera::Camera.Eye -= (Camera::Camera.ViewDirection * Speed);
}

void Camera::MoveLeft(float Speed)
{
	glm::vec3 RightVector = glm::cross(Camera::Camera.ViewDirection, Camera::Camera.UpVector);
	Camera::Camera.Eye -= RightVector * Speed;
}

void Camera::MoveRight(float Speed)
{
	glm::vec3 RightVector = glm::cross(Camera::Camera.ViewDirection, Camera::Camera.UpVector);
	Camera::Camera.Eye += RightVector * Speed;

}

void Camera::MouseLook(int MouseX, int MouseY)
{
	glm::vec2 CurrentMouse = glm::vec2(MouseX, MouseY);

	static bool FirstLook = true;
	if (FirstLook)
	{
		Camera::OldMousePosition = CurrentMouse;
		FirstLook = false;
	}

	glm::vec2 MouseDelta = Camera::OldMousePosition - CurrentMouse;

	/*
		Find a way to make this more efficient
	*/
	

	// Left and right
	Camera::Camera.ViewDirection = glm::rotate(Camera::Camera.ViewDirection, glm::radians(MouseDelta.x), Camera::Camera.UpVector);
	// Up and down
	Camera::Camera.ViewDirection = glm::rotate(Camera::Camera.ViewDirection, glm::radians(MouseDelta.y), glm::cross(Camera::Camera.ViewDirection, Camera::Camera.UpVector));

	Camera::OldMousePosition = CurrentMouse;
}