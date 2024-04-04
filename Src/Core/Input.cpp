#include "Input.h"
#include "../Common.h"

#include "Camera.h"

#include "./API/Vulkan/Renderer.h"

#include "GameObject.h"

SDL_Event Event;

void Engine::Input::ParseEvent()
{
    while (SDL_PollEvent(&Event))
   {
        if (Event.type == SDL_KEYDOWN)
        {
            if (Event.key.keysym.sym == SDLK_SPACE)
            {
                Engine::Camera::MoveUp(0.5f);
            }

            if (Event.key.keysym.sym == SDLK_LCTRL)
            {
                Engine::Camera::MoveDown(0.5f);
            }

            if (Event.key.keysym.sym == SDLK_w)
            {
                Engine::Camera::MoveForward(0.5f);
            }

            if (Event.key.keysym.sym == SDLK_a)
            {
                Engine::Camera::MoveLeft(0.5f);
            }

            if (Event.key.keysym.sym == SDLK_s)
            {
                Engine::Camera::MoveBackward(0.5f);
            }

            if (Event.key.keysym.sym == SDLK_d)
            {
                Engine::Camera::MoveRight(0.5f);
            }

            if (Event.key.keysym.sym == SDLK_ESCAPE)
            {
                Vulkan::Renderer::CleanUp();
            }

            if (Event.key.keysym.sym == SDLK_h)
            {
               // std::cout << "PRESSING H" << std::endl;

                Engine::GameObject::Object Object = Engine::GameObject::GetGameObject();

                Engine::GameObject::UpdateGameObjectRotation(Object, 5.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                //Engine::GameObject::UpdateGameObjectRotation(Object, 15.0f, glm::vec3(0.0f, 1.0f, 0.0f));

               // std::cout << glm::to_string(Object.Transform) << std::endl;

              //  std::cout << "UPDATING POSITION: " << glm::to_string(obj.Position) << std::endl;

            }
        }

        if (Event.type == SDL_MOUSEMOTION)
        {
            Engine::Input::ParseMouseMotion(Event.motion.xrel / 3, Event.motion.yrel / 3);
        }
    }
}

void Engine::Input::ParseMouseMotion(int MouseX, int MouseY)
{
	Engine::Input::MouseX += MouseX;
	Engine::Input::MouseY += MouseY;

	Engine::Camera::MouseLook(Engine::Input::MouseX, Engine::Input::MouseY);
}