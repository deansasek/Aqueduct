**Vulkan Tutorial Project**

Vulkan project based upon https://vulkan-tutorial.com/, with minor changes to the guide:
- Functions & variables changed from lower camel case to pascal case.
- Common headers moved into a "Common" header file to reduce bloat.
- All engine specific functions moved into an "Engine" namespace, with an unused "Renderer" namespace.
- Switched GLFW for SDL2.

Project dependencies:
- SDL2
- GLM

To come:
- [ ] Vertex & uniform buffer support.
- [ ] Texture mapping.
- [ ] Depth buffering.
- [ ] Loading .OBJ, .FBX, and .GLTF models.
- [ ] DearImGui integration.
