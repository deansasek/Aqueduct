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
                UniqueVertices[Vertex] = static_cast<uint32_t>(Vulkan::Renderer::Vertices.size());
                Vulkan::Renderer::Vertices.push_back(Vertex);

            }
            Vulkan::Renderer::Indices.push_back(UniqueVertices[Vertex]);
        }
    }
}