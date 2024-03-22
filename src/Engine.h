#pragma once

namespace Engine
{
	class Application;

	const uint32_t MaxFramesInFlight = 2;

	const std::vector<const char*> ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensions = {
		"VK_KHR_SWAPCHAIN_EXTENSION_NAME"
	};

	#ifdef NDEBUG
		const bool EnableValidationLayers = false;
	#else
		const bool EnableValidationLayers = true;
	#endif

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool isComplete()
		{
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	void FrameBufferResizeCallback(SDL_Window Window);

	void Run();

	void Init();
	void InitWindow();
	void InitVulkan();

	void CreateInstance();
	void InitDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* PAllocator, VkDebugUtilsMessengerEXT* PDebugMessenger);
	void PopulateDebugMessengerCreateInfo(auto VkDebugUtilsMessengerCreateInfoEXT &CreateInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* PAllocator);

	void Cleanup();
	

	void CreateSwapChain();
	void RecreateSwapChain();
	void CleanupSwapChain();
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

	void CreateSurface();

	int RateDeviceSuitability(VkPhysicalDevice Device);
	void PickPhysicalDevice();
	void CreateLogicalDevice();

	void CreateCommandBuffers();
	void CreateCommandPool();
	void RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t ImageIndex);

	void CreateFramebuffers();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateImageViews();

	void CreateSyncObjects();

	void DrawFrame();

	bool CheckValidationLayerSupport();

	bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);

	static std::vector<char> ReadFile(const std::string& filename);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);

	std::vector<const char*> GetRequiredExtensions();

	VkShaderModule CreateShaderModule(const std::vector<char>& Code);
	
	const char* WindowName = "Aqueduct";
	int WindowWidth = 1920, int WindowHeight = 1080;

	SDL_Window* Window;
	VkInstance Instance;

	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkDevice Device;

	VkQueue GraphicsQueue;
	VkQueue PresentQueue;

	VkSurfaceKHR Surface;
	VkDebugUtilsMessengerEXT DebugMessenger;

	VkSwapchainKHR SwapChain;
	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;

	VkRenderPass RenderPass;

	VkPipelineLayout PipelineLayout;
	VkPipeline GraphicsPipeline;

	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;

	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> InFlightFences;

	uint32_t CurrentFrame = 0;

	SDL_bool WindowFullscreen;

	bool WindowMinimized = false;

	bool FramebufferResized = false;

	bool IsRunning = false;
}