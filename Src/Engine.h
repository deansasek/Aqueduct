#pragma once

#ifndef ENGINE_H
#define ENGINE_H

namespace Engine
{
	class Application;

	extern enum API EngineAPI;

	void Run();
	void Init(API API);
	void SetAPI(API API);

	const API& GetAPI();

	extern const char* WindowName;
	
	extern int WindowWidth;
	extern int WindowHeight;

	extern bool WindowMinimized;
	extern bool Running;

	extern SDL_bool WindowFullscreen;
}

#endif