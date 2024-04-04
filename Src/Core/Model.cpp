#include <unordered_map>

#include "../Common.h"
#include "./API/Vulkan/Renderer.h"
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

std::string Engine::Model::TexturePath = "Assets/Textures/model.png";

namespace std
{
    template<> struct hash<Vulkan::Renderer::Vertex>
    {
        //size_t Vulkan::Renderer::Vertex.Operator()(Vulkan::Renderer::Vertex const& Vertex) const
        size_t operator()(Vulkan::Renderer::Vertex const& Vertex) const
        {
            return ((hash<glm::vec3>()(Vertex.Pos) ^
                (hash<glm::vec3>()(Vertex.Color) << 1)) >> 1) ^
                (hash<glm::vec2>()(Vertex.UV) << 1);
        }
    };
}

void Engine::Model::Load(std::string FilePath)
{
    std::cout << "LOADING MODEL!" << std::endl;

    Engine::Model::LoadModel(FilePath);

    Engine::Model::CreateVertexBuffer();
    Engine::Model::CreateIndexBuffer();
}

void Engine::Model::Render(VkCommandBuffer CommandBuffer)
{
    VkBuffer VertexBuffers[] = { Engine::Model::VertexBuffer };
    VkDeviceSize Offsets[] = { 0 };
    vkCmdBindVertexBuffers(CommandBuffer, 0, 1, VertexBuffers, Offsets);
    vkCmdBindIndexBuffer(CommandBuffer, Engine::Model::IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(Engine::Model::Indices.size()), 1, 0, 0, 0);
}

void Engine::Model::Destroy()
{
    vkDestroyBuffer(Vulkan::Renderer::Device, VertexBuffer, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, VertexBufferMemory, nullptr);

    vkDestroyBuffer(Vulkan::Renderer::Device, IndexBuffer, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, IndexBufferMemory, nullptr);
}

void Engine::Model::LoadModel(std::string ModelPath)
{
    tinyobj::attrib_t Attrib;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;
    std::string Warn, Error;

    if (!tinyobj::LoadObj(&Attrib, &Shapes, &Materials, &Warn, &Error, ModelPath.c_str()))
    {
        throw std::runtime_error(Warn + Error);
    }
    else
    {
        std::cout << "VK > Successfully loaded model!" << std::endl;
    }

    std::unordered_map<Vulkan::Renderer::Vertex, uint32_t> UniqueVertices{};

    for (const auto& Material : Materials) {
        std::cout << Material.diffuse_texname << std::endl;
    }

    for (const auto& Shape : Shapes)
    {
        for (const auto& Index : Shape.mesh.indices)
        {
            Vulkan::Renderer::Vertex Vertex{};

            if (Index.vertex_index >= 0)
            {
                Vertex.Pos = {
                    Attrib.vertices[3 * Index.vertex_index + 0],
                    Attrib.vertices[3 * Index.vertex_index + 1],
                    Attrib.vertices[3 * Index.vertex_index + 2]
                };
            }

            auto ColorIndex = 3 * Index.vertex_index + 2;
            if (ColorIndex < Attrib.colors.size())
            {
                Vertex.Color = {
                    Attrib.colors[ColorIndex - 2],
                    Attrib.colors[ColorIndex - 1],
                    Attrib.colors[ColorIndex - 0]
                };
            }
            else
            {
                Vertex.Color = { 1.0f, 1.0f, 1.0f };
            }

            if (Index.normal_index >= 0)
            {
                Vertex.Normal = {
                    Attrib.normals[3 * Index.normal_index + 0],
                    Attrib.normals[3 * Index.normal_index + 1],
                    Attrib.normals[3 * Index.normal_index + 2]
                };
            }

            if (Index.texcoord_index >= 0)
            {
                Vertex.UV = {
                    Attrib.texcoords[2 * Index.texcoord_index + 0],
                    1.0f - Attrib.texcoords[2 * Index.texcoord_index + 1]
                };
            }

            if (UniqueVertices.count(Vertex) == 0)
            {
                UniqueVertices[Vertex] = static_cast<uint32_t>(Engine::Model::Vertices.size());
                Engine::Model::Vertices.push_back(Vertex);

            }
            Engine::Model::Indices.push_back(UniqueVertices[Vertex]);
        }
    }

    std::cout << "VERTICES COUNT: " << Engine::Model::Vertices.size() << std::endl;
    std::cout << "INDICES COUNT: " << Engine::Model::Indices.size() << std::endl;
}

void Engine::Model::CreateVertexBuffer()
{
    VkDeviceSize BufferSize = sizeof(Engine::Model::Vertices[0]) * Engine::Model::Vertices.size();

    std::cout << "VERTEX BUFFER SIZE: " << BufferSize << std::endl;
    std::cout << "VERTICES SIZE 2: " << Engine::Model::Vertices.size() << std::endl;

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;

    Vulkan::Renderer::CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* Data;
    vkMapMemory(Vulkan::Renderer::Device, StagingBufferMemory, 0, BufferSize, 0, &Data);
    memcpy(Data, Engine::Model::Vertices.data(), (size_t)BufferSize);
    vkUnmapMemory(Vulkan::Renderer::Device, StagingBufferMemory);

    Vulkan::Renderer::CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Engine::Model::VertexBuffer, Engine::Model::VertexBufferMemory);

    Vulkan::Renderer::CopyBuffer(StagingBuffer, Engine::Model::VertexBuffer, BufferSize);

    vkDestroyBuffer(Vulkan::Renderer::Device, StagingBuffer, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, StagingBufferMemory, nullptr);
}

void Engine::Model::CreateIndexBuffer()
{
    VkDeviceSize BufferSize = sizeof(Engine::Model::Indices[0]) * Engine::Model::Indices.size();

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;

    Vulkan::Renderer::CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* Data;
    vkMapMemory(Vulkan::Renderer::Device, StagingBufferMemory, 0, BufferSize, 0, &Data);
    memcpy(Data, Engine::Model::Indices.data(), (size_t)BufferSize);
    vkUnmapMemory(Vulkan::Renderer::Device, StagingBufferMemory);

    Vulkan::Renderer::CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Engine::Model::IndexBuffer, Engine::Model::IndexBufferMemory);

    Vulkan::Renderer::CopyBuffer(StagingBuffer, Engine::Model::IndexBuffer, BufferSize);

    vkDestroyBuffer(Vulkan::Renderer::Device, StagingBuffer, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, StagingBufferMemory, nullptr);
}