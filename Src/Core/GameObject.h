#pragma once

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Model.h"

namespace Engine
{
	class GameObject
	{
		public:
			struct Object
			{
				Engine::Model Model;
				//glm::vec3 Model;
				std::string ModelPath;
				glm::vec3 Position;
				glm::vec3 Rotation;
				glm::vec3 Scale;
				glm::mat4 Transform;
			};

			static std::vector<Object> GameObjects;

			Object CreateGameObject(std::string FilePath, glm::vec3 Position, glm::vec3 Scale);

			//static void SetGameObjectPosition(Object& GameObject, glm::vec3 Position);
			//static void SetGameObjectRotation(Object& GameObject, float Radians, glm::vec3 RotationDirection);
			//static void SetGameObjectScale(Object& GameObject, glm::vec3 Scale);

			static void UpdateGameObjectPosition(Object& GameObject, glm::vec3 Position);
			static void UpdateGameObjectRotation(Object& GameObject, float Radians, glm::vec3 RotationDirection);
			static void UpdateGameObjectScale(Object& GameObject, glm::vec3 Scale);

			static void CreateGameObjects();
			static void RenderGameObject(Object GameObject);
			static void RenderGameObjects(VkCommandBuffer CommandBuffer);

			static Engine::GameObject::Object GetGameObject();

	};
}

#endif