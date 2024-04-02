#include "Common.h"
#include "Engine.h"
#include "Core/FileSystem/FileSystem.h"

#include "Core/Input.h"
#include "Core/Camera.h"

#include "Core/API/Vulkan/Renderer.h"

bool Engine::Running;

API Engine::EngineAPI = API::Undefined;

SDL_bool WindowFullscreen;

void Engine::Run()
{
    Engine::Init(API::Vulkan);
    FileSystem::LoadTextures();

    Engine::Running = true;

    SDL_WarpMouseInWindow(Vulkan::Renderer::Window, Vulkan::Renderer::SwapChainExtent.width / 2, Vulkan::Renderer::SwapChainExtent.height / 2);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    //Input::MouseX = Vulkan::Renderer::SwapChainExtent.width / 2;
    //Input::MouseY = Vulkan::Renderer::SwapChainExtent.height / 2;

    while (Engine::Running)
    {
        SDL_Event RunEvent;

        Engine::Input::ParseEvent();

        /*
            Input Events
        */

        while (SDL_PollEvent(&RunEvent))
        {
            if (RunEvent.type == SDL_QUIT)
            {
                Engine::Running = false;
                break;
            }
        }

        /*
            Drawing Events
        */

        if (Engine::GetAPI() == API::Vulkan)
        {
            Vulkan::Renderer::DrawFrame();
        }
        else if (Engine::GetAPI() == API::DirectX12)
        {
            throw std::runtime_error("ENGINE > Unsupported graphics API selected!");
        }
        else if (Engine::GetAPI() == API::OpenGL)
        {
            throw std::runtime_error("ENGINE > Unsupported graphics API selected!");
        }
    }
}

void Engine::SetAPI(API EngineAPI)
{
    Engine::EngineAPI = EngineAPI;
}

const API& Engine::GetAPI()
{
    return Engine::EngineAPI;
}

void Engine::Init(API API)
{
    Engine::SetAPI(API::Vulkan);

    /*
        Vulkan API
    */

    if (Engine::GetAPI() == API::Vulkan)
    {
        Vulkan::Renderer::InitWindow();
        Vulkan::Renderer::Init();

        Camera::CreateCamera();
    }

    /*
        DirectX 12 API
    */

    else if (Engine::GetAPI() == API::DirectX12)
    {
        std::cout << "No DirectX12 Support! \n";
    }

    /*
        OpenGL API
    */

    else if (Engine::GetAPI() == API::OpenGL)
    {
        std::cout << "No OpenGL Support! \n";
    }
    else
    {
        throw std::runtime_error("Unrecognized API passed into engine initialization! \n");
    }
}