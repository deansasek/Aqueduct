#include "../Common.h"
#include "./API/Vulkan/Renderer.h"
#include "GameObject.h"
#include "Model.h"

std::vector<Engine::GameObject::Object> Engine::GameObject::GameObjects;

Engine::GameObject::Object Engine::GameObject::CreateGameObject(std::string FilePath, glm::vec3 Position, glm::vec3 Scale)
{
	Engine::Model Model;

	Model.Load(FilePath);

	Engine::GameObject::Object GameObject{};
	GameObject.Model = Model;
	GameObject.ModelPath = FilePath;
	GameObject.Position = Position;
	GameObject.Scale = Scale;

	glm::mat4 Transform;

	Transform = glm::translate(glm::mat4(1.0f), GameObject.Position);
	Transform = glm::scale(Transform, GameObject.Scale);
	GameObject.Transform = Transform;

	Engine::GameObject::GameObjects.push_back(GameObject);

	return GameObject;
}

void Engine::GameObject::UpdateGameObjectPosition(Engine::GameObject::Object& GameObject, glm::vec3 Position)
{
	GameObject.Position = Position;

	for (int i = 0; i < Engine::GameObject::GameObjects.size(); i++)
	{
		/*
			Rewrite this to be more abstract
		*/
		if (Engine::GameObject::GameObjects[i].ModelPath == GameObject.ModelPath)
		{
			GameObject.Transform = glm::translate(GameObject.Transform, Position);

			Engine::GameObject::GameObjects[i].Transform = GameObject.Transform;
		}
	}
}

void Engine::GameObject::UpdateGameObjectRotation(Engine::GameObject::Object& GameObject, float Radians, glm::vec3 RotationDirection)
{
	for (int i = 0; i < Engine::GameObject::GameObjects.size(); i++)
	{
		/*
			Rewrite this to be more abstract
		*/
		if (Engine::GameObject::GameObjects[i].ModelPath == GameObject.ModelPath)
		{
			GameObject.Transform = glm::rotate(GameObject.Transform, glm::radians(Radians), RotationDirection);

			Engine::GameObject::GameObjects[i].Transform = GameObject.Transform;
		}
	}
}

void Engine::GameObject::UpdateGameObjectScale(Engine::GameObject::Object& GameObject, glm::vec3 Scale)
{
	GameObject.Scale = Scale;

	for (int i = 0; i < Engine::GameObject::GameObjects.size(); i++)
	{
		/*
			Rewrite this to be more abstract
		*/
		if (Engine::GameObject::GameObjects[i].ModelPath == GameObject.ModelPath)
		{
			GameObject.Transform = glm::scale(GameObject.Transform, Scale);

			Engine::GameObject::GameObjects[i].Transform = GameObject.Transform;
		}
	}
}

/*
void Engine::GameObject::UpdateGameObject(Engine::GameObject::Object& GameObject, glm::vec3 Position, glm::vec3 Scale, float Radians, glm::vec3 RotationDirection)
{
	GameObject.Position = Position;

	for (int i = 0; i < Engine::GameObject::GameObjects.size(); i++)
	{
		if (Engine::GameObject::GameObjects[i].ModelPath == GameObject.ModelPath)
		{
			GameObject.Transform = GameObject.Transform += glm::translate(GameObject.Transform, Position);
			GameObject.Transform = GameObject.Transform += glm::rotate(GameObject.Transform, glm::radians(Radians), RotationDirection);
			GameObject.Transform = GameObject.Transform += glm::scale(GameObject.Transform, Scale);

			Engine::GameObject::GameObjects[i].Transform = GameObject.Transform;
		}
	}
}
*/

void Engine::GameObject::CreateGameObjects()
{
	Engine::GameObject GameObject;

	GameObject.CreateGameObject("Assets/Models/280z.obj", glm::vec3(23.0f, -14.0f, 10.0f), glm::vec3(4.0f));
	GameObject.CreateGameObject("Assets/Models/de_dust2.obj", glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(5.0f));
}

void Engine::GameObject::RenderGameObject(Engine::GameObject::Object GameObject)
{
}

void Engine::GameObject::RenderGameObjects(VkCommandBuffer CommandBuffer)
{
	for (auto& GameObject : Engine::GameObject::GameObjects)
	{
		vkCmdPushConstants(CommandBuffer, Vulkan::Renderer::PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &GameObject.Transform);

		GameObject.Model.Render(CommandBuffer);
	}
}

Engine::GameObject::Object Engine::GameObject::GetGameObject()
{
	return Engine::GameObject::GameObjects[0];
}