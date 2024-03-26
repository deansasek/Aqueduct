#pragma once

namespace Vulkan
{
	namespace Renderer
	{
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> GraphicsFamily;
			std::optional<uint32_t> PresentFamily;

			bool IsComplete()
			{
				return GraphicsFamily.has_value() && PresentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR Capabilities;
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;
		};

		struct Vertex {
			glm::vec2 Pos;
			glm::vec3 Color;

			static VkVertexInputBindingDescription GetBindingDescription()
			{
				VkVertexInputBindingDescription BindingDescription{};
				BindingDescription.binding = 0;
				BindingDescription.stride = sizeof(Vertex);
				BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return BindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 2> AttributeDescriptions{};
				AttributeDescriptions[0].binding = 0;
				AttributeDescriptions[0].location = 0;
				AttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
				AttributeDescriptions[0].offset = offsetof(Vertex, Pos);

				AttributeDescriptions[1].binding = 0;
				AttributeDescriptions[1].location = 1;
				AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				AttributeDescriptions[1].offset = offsetof(Vertex, Color);

				return AttributeDescriptions;
			}
		};

		struct UniformBufferObject {
			alignas(16) glm::mat4 Model;
			alignas(16) glm::mat4 View;
			alignas(16) glm::mat4 Proj;
		};

		extern const std::vector<Vertex> Vertices;
		extern const std::vector<uint16_t> Indices;

		const std::vector<const char*> ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		#ifdef NDEBUG
				const bool EnableValidationLayers = false;
		#else
				const bool EnableValidationLayers = true;
		#endif

		extern SDL_Window* Window;
		extern VkInstance Instance;
		extern VkSurfaceKHR Surface;

		extern VkPhysicalDevice PhysicalDevice;
		extern VkDevice Device;

		extern VkQueue GraphicsQueue;
		extern VkQueue PresentQueue;

		extern VkDescriptorSetLayout DescriptorSetLayout;
		extern VkPipelineLayout PipelineLayout;
		extern VkPipeline GraphicsPipeline;

		extern VkSwapchainKHR SwapChain;
		extern std::vector<VkImage> SwapChainImages;
		extern VkFormat SwapChainImageFormat;
		extern VkExtent2D SwapChainExtent;
		extern std::vector<VkImageView> SwapChainImageViews;
		extern std::vector<VkFramebuffer> SwapChainFramebuffers;

		extern VkRenderPass RenderPass;

		extern VkCommandPool CommandPool;
		extern std::vector<VkCommandBuffer> CommandBuffers;

		extern std::vector<VkSemaphore> ImageAvailableSemaphores;
		extern std::vector<VkSemaphore> RenderFinishedSemaphores;
		extern std::vector<VkFence> InFlightFences;

		extern VkDebugUtilsMessengerEXT DebugMessenger;

		extern VkBuffer VertexBuffer;
		extern VkDeviceMemory VertexBufferMemory;

		extern VkBuffer IndexBuffer;
		extern VkDeviceMemory IndexBufferMemory;

		extern std::vector<VkBuffer> UniformBuffers;
		extern std::vector<VkDeviceMemory> UniformBuffersMemory;
		extern std::vector<void*> UniformBuffersMapped;

		extern VkDescriptorPool DescriptorPool;
		extern std::vector<VkDescriptorSet> DescriptorSets;

		extern bool FramebufferResized;

		const uint32_t MaxFramesInFlight = 2;

		extern uint32_t CurrentFrame;

		void Init();
		void InitDebugMessenger();
		void InitWindow();
		void CleanUp();
		void CreateSurface();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void RecreateSwapChain();
		void CleanUpSwapChain();
		void CreateImageViews();
		void CreateFramebuffers();
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateGraphicsPipeline();
		void CreateRenderPass();
		void CreateCommandBuffers();
		void CreateCommandPool();
		void RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t ImageIndex);
		void CreateSyncObjects();
		void DrawFrame();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo);
		void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* PAllocator);
		void CreateTextureImage();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void UpdateUniformBuffer(uint32_t CurrentImage);
		void CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory);
		void CopyBuffer(VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);
		bool CheckValidationLayerSupport();
		bool IsDeviceSuitable(VkPhysicalDevice Device);

		int RateDeviceSuitability(VkPhysicalDevice Device);

		std::vector<const char*> GetRequiredExtensions();

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

		VkShaderModule CreateShaderModule(const std::vector<char>& Code);

		uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* PAllocator, VkDebugUtilsMessengerEXT* PDebugMessenger);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* PCallbackData, void* PUserData);
	}
}