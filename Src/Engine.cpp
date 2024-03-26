#include "Common.h"
#include "Engine.h"

#include "Core/Input.h"
#include "Core/Camera.h"

#include "Core/API/Vulkan/Renderer.h"

bool Engine::Running;

API Engine::EngineAPI = API::Undefined;

SDL_bool WindowFullscreen;

void Engine::Run()
{
    Engine::Init(API::Vulkan);

    Engine::Running = true;

    SDL_WarpMouseInWindow(Vulkan::Renderer::Window, Vulkan::Renderer::SwapChainExtent.width / 2, Vulkan::Renderer::SwapChainExtent.height / 2);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    //Input::MouseX = Vulkan::Renderer::SwapChainExtent.width / 2;
    //Input::MouseY = Vulkan::Renderer::SwapChainExtent.height / 2;

    while (Engine::Running)
    {
        SDL_Event RunEvent;
        SDL_Event EngineEvent;

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

            while (SDL_PollEvent(&EngineEvent))
            {
                if (EngineEvent.type == SDL_KEYDOWN)
                {
                    if (EngineEvent.key.keysym.sym == SDLK_w)
                    {
                        Camera::MoveForward(0.1f);
                    }

                    if (EngineEvent.key.keysym.sym == SDLK_a)
                    {
                        Camera::MoveLeft(0.1f);
                    }

                    if (EngineEvent.key.keysym.sym == SDLK_s)
                    {
                        Camera::MoveBackward(0.1f);
                    }

                    if (EngineEvent.key.keysym.sym == SDLK_d)
                    {
                        Camera::MoveRight(0.1f);
                    }
                }

                if (EngineEvent.type == SDL_MOUSEMOTION)
                {
                    Input::ParseMouseMotion(EngineEvent.motion.xrel / 3, EngineEvent.motion.yrel / 3);
                }
            }
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







/*
void Engine::Run()
{
    std::cout << "Engine running \n";

    Engine::Init(API::Vulkan);

    Engine::IsRunning = true;

    while (Engine::IsRunning)
    {
        SDL_Event WindowEvent;

        while (SDL_PollEvent(&WindowEvent))
            if (WindowEvent.type == SDL_QUIT)
            {
                Engine::IsRunning = false;
                break;
            }
            else if (WindowEvent.type == SDL_WINDOWEVENT && WindowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                Engine::FrameBufferResizeCallback(Engine::Window);
            }
            else if (WindowEvent.type == SDL_WINDOWEVENT && WindowEvent.window.event == SDL_WINDOWEVENT_MINIMIZED)
            {
                Engine::WindowMinimized = true;
            }
            else if (WindowEvent.type == SDL_WINDOWEVENT && WindowEvent.window.event == SDL_WINDOWEVENT_RESTORED)
            {
                Engine::WindowMinimized = false;
            }

        if (!Engine::WindowMinimized)
        {
            if (Engine::GetAPI() == API::Vulkan)
            {
                Engine::DrawFrame();
            }
        }
    }

    vkDeviceWaitIdle(Engine::Device);

    Engine::Cleanup();
}
*/