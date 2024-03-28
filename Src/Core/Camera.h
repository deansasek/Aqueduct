#pragma once

#ifndef CAMERA_HH
#define CAMERA_H

namespace Camera
{
	struct CameraStruct {
		glm::vec3 Eye;
		glm::vec3 ViewDirection;
		glm::vec3 UpVector;
	};

	extern CameraStruct Camera;

	extern glm::vec2 OldMousePosition;

	void CreateCamera();
	glm::mat4 GetViewMatrix();

	void MoveUp(float Speed);
	void MoveDown(float Speed);
	void MoveForward(float Speed);
	void MoveBackward(float Speed);
	void MoveLeft(float Speed);
	void MoveRight(float Speed);

	void MouseLook(int MouseX, int MouseY);
}

#endif