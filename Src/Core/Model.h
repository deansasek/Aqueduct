#pragma once

#ifndef MODEL_H
#define MODEL_H

namespace Engine
{
	class Model
	{
	public:
		std::vector<Vulkan::Renderer::Vertex> Vertices;
		std::vector<uint32_t> Indices;
		VkBuffer VertexBuffer;
		VkDeviceMemory VertexBufferMemory;
		VkBuffer IndexBuffer;
		VkDeviceMemory IndexBufferMemory;

		static std::string TexturePath;

		void Load(std::string FilePath);
		void Render(VkCommandBuffer CommandBuffer);
		void Destroy();
	private:
		void LoadModel(std::string FilePath);
		void CreateVertexBuffer();
		void CreateIndexBuffer();
	};
}

#endif