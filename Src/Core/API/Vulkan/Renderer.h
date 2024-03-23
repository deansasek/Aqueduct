#pragma once

#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

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
		void CreateGraphicsPipeline();
		void CreateRenderPass();
		void CreateCommandBuffers();
		void CreateCommandPool();
		void RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t ImageIndex);
		void CreateSyncObjects();
		void DrawFrame();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo);
		void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* PAllocator);

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

		VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* PAllocator, VkDebugUtilsMessengerEXT* PDebugMessenger);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* PCallbackData, void* PUserData);

		/*
			File System Module
		*/
		std::vector<char> ReadFile(const std::string& FileName);
	}
}

#endif