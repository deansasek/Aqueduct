#pragma once

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

namespace Engine
{
	namespace GameObject
	{
		struct GameObject {
			std::string Name;
			std::string ModelPath;
		};

		extern std::vector<GameObject> GameObjects;

		void CreateGameObject(std::string Name, std::string ModelPath);
		void CreateGameObjects();
	}
}

#endif