**Vulkan Starter Project**

Vulkan project based upon https://vulkan-tutorial.com/, with minor changes to the guide:
- Pascal case functions & variables
- Common headers moved into a "Common" header file to reduce bloat
- All engine specific functions moved into an "Engine" namespace, with an unused "Renderer" namespace
- Switched GLFW for SDL2
