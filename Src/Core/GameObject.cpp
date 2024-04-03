#include "../Common.h"
#include "./API/Vulkan/Renderer.h"
#include "Model.h"
#include "GameObject.h"

std::vector<Engine::GameObject::GameObject> Engine::GameObject::GameObjects;

void Engine::GameObject::CreateGameObject(std::string Name, std::string ModelPath)
{
	Engine::GameObject::GameObject Object{};
	Object.Name = Name;
	Object.ModelPath = ModelPath;

	Engine::GameObject::GameObjects.push_back(Object);
}

void Engine::GameObject::CreateGameObjects()
{
	Engine::GameObject::CreateGameObject("hi", "Assets/Models/280z.obj");
	Engine::GameObject::CreateGameObject("hi", "Assets/Models/de_dust2.obj");

	for (auto& GameObject : Engine::GameObject::GameObjects)
	{
		std::cout << GameObject.Name << " loaded!" << std::endl;

		Engine::Model::LoadModel(GameObject.ModelPath);
	}
}