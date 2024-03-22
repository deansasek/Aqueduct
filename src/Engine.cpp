#include "Engine.h"
#include "Common.h"

void Engine::Run()
{
	std::cout << "Engine running \n";

    Engine::Init();
    
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
            Engine::DrawFrame();
        }
    }

    vkDeviceWaitIdle(Engine::Device);

    Engine::Cleanup();
}

void Engine::Init()
{
	std::cout << "Engine initialized \n";

    Engine::InitWindow();
    Engine::InitVulkan();
}

void Engine::InitWindow()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Vulkan_LoadLibrary(nullptr);

	Engine::Window = SDL_CreateWindow(Engine::WindowName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Engine::WindowWidth, Engine::WindowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowData(Engine::Window, "SDL_Window", &Engine::Window);
}

void Engine::InitVulkan()
{
    Engine::CreateInstance();
    Engine::InitDebugMessenger();
    Engine::CreateSurface();
    Engine::PickPhysicalDevice();
    Engine::CreateLogicalDevice();
    Engine::CreateSwapChain();
    Engine::CreateImageViews();
    Engine::CreateRenderPass();
    Engine::CreateGraphicsPipeline();
    Engine::CreateFramebuffers();
    Engine::CreateCommandPool();
    Engine::CreateCommandBuffers();
    Engine::CreateSyncObjects();
}

bool Engine::CheckDeviceExtensionSupport(VkPhysicalDevice Device)
{
    uint32_t ExtensionCount;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr);

    std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, AvailableExtensions.data());

    std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& Extension : AvailableExtensions)
    {
        RequiredExtensions.erase(Extension.extensionName);
    }

    return RequiredExtensions.empty();
}

Engine::QueueFamilyIndices Engine::FindQueueFamilies(VkPhysicalDevice Device)
{
    Engine::QueueFamilyIndices Indices;

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Engine::Device, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Engine::Device, &QueueFamilyCount, QueueFamilies.data());

    int i = 0;

    // loops through queuefamilies
    for (const auto& QueueFamily : QueueFamilies)
    {
        // loops through queueflags in queuefamily to determine if VK_QUEUE_GRAPHICS_BIT is available
        if (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Indices.GraphicsFamily = i;
        }

        VkBool32 PresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(Engine::Device, i, Engine::Device, &PresentSupport);

        if (PresentSupport)
        {
            Indices.PresentFamily = i;
        }

        if (Indices.isComplete())
        {
            break;
        }

        i++;
    }

    return Indices;
}

void Engine::CreateInstance()
{
    if (EnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested, but not available");
    }

    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Hello Triangle";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "No Engine";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    auto Extensions = GetRequiredExtensions();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
    CreateInfo.ppEnabledExtensionNames = Extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

    if (EnableValidationLayers)
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = ValidationLayers.data();

        PopulateDebugMessengerCreateInfo(DebugCreateInfo);
        CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
        CreateInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&CreateInfo, nullptr, &Engine::Instance);

    if (vkCreateInstance(&CreateInfo, nullptr, &Engine::Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance");
    }
    else if (vkCreateInstance(&CreateInfo, nullptr, &Engine::Instance) == VK_SUCCESS)
    {
        std::cout << "Successfully created instance \n";
    }
}

void Engine::InitDebugMessenger()
{
    if (!EnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
    PopulateDebugMessengerCreateInfo(CreateInfo);

    if (CreateDebugUtilsMessengerEXT(Engine::Instance, &CreateInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create debug messenger");
    }
    else if (CreateDebugUtilsMessengerEXT(Engine::Instance, &CreateInfo, nullptr, &DebugMessenger) == VK_SUCCESS)
    {
        std::cout << "Successfully created debug messenger \n";
    }
}

VkResult Engine::CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* PAllocator, VkDebugUtilsMessengerEXT* PDebugMessenger)
{
    auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");

    if (Func != nullptr) {
        return Func(Instance, PCreateInfo, PAllocator, PDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Engine::DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* PAllocator)
{
    auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");

    if (Func != nullptr)
    {
        Func(Instance, DebugMessenger, PAllocator);
    }
}

void Engine::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &CreateInfo)
{
    CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    CreateInfo.pfnUserCallback = DebugCallback;
    CreateInfo.pUserData = nullptr;
}

bool Engine::CheckValidationLayerSupport()
{
    uint32_t LayerCount;
    vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

    std::vector<VkLayerProperties> AvailableLayers(LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

    for (const char* LayerName : ValidationLayers)
    {
        bool LayerFound = false;

        for (const auto& LayerProperties : AvailableLayers)
        {
            if (strcmp(LayerName, LayerProperties.layerName) == 0)
            {
                LayerFound = true;
                break;
            }
        }

        if (!LayerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> Engine::GetRequiredExtensions()
{
    uint32_t SDL2ExtensionCount;

    const char** SDL2ExtensionNames;

    SDL_Vulkan_GetInstanceExtensions(Engine::Window, &SDL2ExtensionCount, nullptr);

    SDL2ExtensionNames = new const char* [SDL2ExtensionCount];

    SDL_Vulkan_GetInstanceExtensions(Engine::Window, &SDL2ExtensionCount, SDL2ExtensionNames);

    std::vector<const char*> SLD2Extensions(SDL2ExtensionNames, SDL2ExtensionNames + SDL2ExtensionCount);

    if (EnableValidationLayers)
    {
        SLD2Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return SLD2Extensions;
}

void Engine::CreateSurface()
{
    if (SDL_Vulkan_CreateSurface(Engine::Window, Engine::Instance, &Engine::Surface) == SDL_FALSE)
    {
        throw std::runtime_error("Failed to create window surface");
    }
    else if (SDL_Vulkan_CreateSurface(Engine::Window, Engine::Instance, &Engine::Surface) == SDL_TRUE)
    {
        std::cout << "Successfully created window surface \n";
    }
}

int Engine::RateDeviceSuitability(VkPhysicalDevice Device)
{
    int Score = 0;

    VkPhysicalDeviceProperties DeviceProperties;
    VkPhysicalDeviceFeatures DeviceFeatures;

    vkGetPhysicalDeviceProperties(Device, &DeviceProperties);
    vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

    if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        Score += 1000;
    }

    Score += DeviceProperties.limits.maxImageDimension2D;

    if (!DeviceFeatures.geometryShader)
    {
        return 0;
    }

    bool ExtensionsSupported = CheckDeviceExtensionSupport(Device);

    bool SwapChainAdequate = false;

    if (ExtensionsSupported)
    {
        SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(Device);
        SwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentModes.empty();
    }

    return Score && ExtensionsSupported && SwapChainAdequate;
}

void Engine::PickPhysicalDevice()
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Engine::Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("No GPU detected with Vulkan support");
    }
    else
    {
        std::cout << "Detected GPU with Vulkan support \n";
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Engine::Instance, &DeviceCount, Devices.data());

    std::multimap<int, VkPhysicalDevice> Candidates;

    // loop through devices 
    for (const auto& Device : Devices)
    {
        int Score = RateDeviceSuitability(Device);
        Candidates.insert(std::make_pair(Score, Device));
    }

    if (Candidates.rbegin()->first > 0)
    {
        PhysicalDevice = Candidates.rbegin()->second;

        QueueFamilyIndices indices = FindQueueFamilies(PhysicalDevice);

        if (indices.isComplete())
        {
            std::cout << "Successfully picked GPU with Vulkan support \n";
        }
    }
    else
    {
        throw std::runtime_error("Failed to locate GPU with Vulkan support");
    }

    if (PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to locate GPU with Vulkan support");
    }
}

void Engine::CreateLogicalDevice()
{
    QueueFamilyIndices Indices = FindQueueFamilies(PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32_t> UniqueQueueFamilies = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    float QueuePriority = 1.0f;

    for (uint32_t QueueFamily : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = QueueFamily;
        QueueCreateInfo.queueCount = 1;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;

        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    // we're coming back to this
    VkPhysicalDeviceFeatures DeviceFeatures{};

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());

    CreateInfo.pEnabledFeatures = &DeviceFeatures;

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    // add validation layers if enabled
    if (EnableValidationLayers)
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &Engine::Device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device");
    }
    else if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &Engine::Device) == VK_SUCCESS)
    {
        std::cout << "Successfully created logical device \n";
    }

    vkGetDeviceQueue(Engine::Device, Indices.GraphicsFamily.value(), 0, &GraphicsQueue);
    vkGetDeviceQueue(Engine::Device, Indices.PresentFamily.value(), 0, &PresentQueue);
}

void Engine::CreateSwapChain()
{
    SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(Engine::PhysicalDevice);

    VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
    VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
    VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.Capabilities);

    uint32_t ImageCount = SwapChainSupport.Capabilities.minImageCount + 1;

    if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImageCount >> SwapChainSupport.Capabilities.maxImageCount)
    {
        ImageCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = Surface;
    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices Indices = FindQueueFamilies(PhysicalDevice);
    uint32_t QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.GraphicsFamily != Indices.PresentFamily)
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0;
        CreateInfo.pQueueFamilyIndices = nullptr;
    }

    CreateInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(Engine::Device, &CreateInfo, nullptr, &SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain");
    }
    else
    {
        std::cout << "Successfully created swap chain \n";
    }

    vkGetSwapchainImagesKHR(Engine::Device, SwapChain, &ImageCount, nullptr);
    Engine::SwapChainImages.resize(ImageCount);
    vkGetSwapchainImagesKHR(Engine::Device, SwapChain, &ImageCount, SwapChainImages.data());

    SwapChainImageFormat = SurfaceFormat.format;
    SwapChainExtent = Extent;
}

Engine::SwapChainSupportDetails Engine::QuerySwapChainSupport(VkPhysicalDevice Device)
{
    Engine::SwapChainSupportDetails Details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Details.Capabilities);

    uint32_t FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr);

    if (FormatCount != 0)
    {
        Details.Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Engine::Surface, &FormatCount, Details.Formats.data());
    }

    // gets supported present modes
    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Engine::Surface, &PresentModeCount, nullptr);

    if (FormatCount != 0)
    {
        Details.PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Engine::Surface, &PresentModeCount, Details.PresentModes.data());
    }

    return Details;
}

VkSurfaceFormatKHR Engine::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &AvailableFormats)
{
    for (const auto &AvailableFormat : AvailableFormats)
    {
        if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return AvailableFormat;
        }
    }

    return AvailableFormats[0];
}

VkPresentModeKHR Engine::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &AvailablePresentModes)
{
    for (const auto &AvailablePresentMode : AvailablePresentModes)
    {
        if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return AvailablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &Capabilities)
{
    if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return Capabilities.currentExtent;
    }
    else
    {
        int Width, Height;

        SDL_Vulkan_GetDrawableSize(Engine::Window, &Width, &Height);

        VkExtent2D ActualExtent = {
            static_cast<uint32_t>(Width),
            static_cast<uint32_t>(Height)
        };

        ActualExtent.width = std::clamp(ActualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        ActualExtent.height = std::clamp(ActualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

        return ActualExtent;
    }
}

void Engine::RecreateSwapChain()
{
    vkDeviceWaitIdle(Engine::Device);

    Engine::CleanupSwapChain();

    Engine::CreateSwapChain();
    Engine::CreateImageViews();
    Engine::CreateFramebuffers();
}

void Engine::CreateImageViews()
{
    SwapChainImageViews.resize(SwapChainImages.size());

    for (size_t i = 0; i < SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        CreateInfo.image = SwapChainImages[i];
        CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        CreateInfo.format = SwapChainImageFormat;

        CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        CreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        CreateInfo.subresourceRange.baseMipLevel = 0;
        CreateInfo.subresourceRange.levelCount = 1;
        CreateInfo.subresourceRange.baseArrayLayer = 0;
        CreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(Engine::Device, &CreateInfo, nullptr, &SwapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image views");
        }
        else
        {
            std::cout << "Successfully created image views \n";
        }
    }
}

VkShaderModule Engine::CreateShaderModule(const std::vector<char>& Code)
{
    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = Code.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

    VkShaderModule ShaderModule;

    if (vkCreateShaderModule(Engine::Device, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }
    else
    {
        std::cout << "Successfully created shader module \n";
    }

    return ShaderModule;
}

void Engine::CreateRenderPass()
{
    VkAttachmentDescription ColorAttachment{};
    ColorAttachment.format = SwapChainImageFormat;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentRef{};
    ColorAttachmentRef.attachment = 0;
    ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubPass{};
    SubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubPass.colorAttachmentCount = 1;
    SubPass.pColorAttachments = &ColorAttachmentRef;

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = 1;
    RenderPassInfo.pAttachments = &ColorAttachment;
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &SubPass;
    RenderPassInfo.dependencyCount = 1;
    RenderPassInfo.pDependencies = &Dependency;

    if (vkCreateRenderPass(Engine::Device, &RenderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass");
    }
    else
    {
        std::cout << "Successfully created render pass \n";
    }
}

void Engine::CreateGraphicsPipeline()
{
    auto VertexShaderCode = Engine::ReadFile("shaders/vert.spv");
    auto FragmentShaderCode = Engine::ReadFile("shaders/frag.spv");

    VkShaderModule VertexShaderModule = CreateShaderModule(VertexShaderCode);
    VkShaderModule FragmentShaderModule = CreateShaderModule(FragmentShaderCode);

    VkPipelineShaderStageCreateInfo VertexShaderStageInfo{};
    VertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertexShaderStageInfo.module = VertexShaderModule;
    VertexShaderStageInfo.pName = "main";
    VertexShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo FragmentShaderStageInfo{};
    FragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentShaderStageInfo.module = FragmentShaderModule;
    FragmentShaderStageInfo.pName = "main";
    FragmentShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo ShaderStages[] = { VertexShaderStageInfo, FragmentShaderStageInfo };

    std::vector<VkDynamicState> DynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo DynamicState{};
    DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicState.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
    DynamicState.pDynamicStates = DynamicStates.data();

    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInfo.vertexBindingDescriptionCount = 0;
    VertexInputInfo.pVertexBindingDescriptions = nullptr;
    VertexInputInfo.vertexAttributeDescriptionCount = 0;
    VertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
    InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport Viewport{};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = (float) SwapChainExtent.width;
    Viewport.height = (float) SwapChainExtent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkRect2D Scissor{};
    Scissor.offset = { 0, 0 };
    Scissor.extent = SwapChainExtent;

    VkPipelineViewportStateCreateInfo ViewportState{};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.pViewports = &Viewport;
    ViewportState.scissorCount = 1;
    ViewportState.pScissors = &Scissor;

    VkPipelineRasterizationStateCreateInfo Rasterizer{};
    Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    Rasterizer.depthClampEnable = VK_FALSE;
    Rasterizer.rasterizerDiscardEnable = VK_FALSE;

    Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    Rasterizer.lineWidth = 1.0f;
    Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    Rasterizer.depthBiasEnable = VK_FALSE;
    Rasterizer.depthBiasConstantFactor = 0.0f;
    Rasterizer.depthBiasClamp = 0.0f;
    Rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo Multisampling{};
    Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    Multisampling.sampleShadingEnable = VK_FALSE;
    Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    Multisampling.minSampleShading = 1.0f;
    Multisampling.pSampleMask = nullptr;
    Multisampling.alphaToCoverageEnable = VK_FALSE;
    Multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_TRUE;
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo ColorBlending{};
    ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlending.logicOpEnable = VK_FALSE;
    ColorBlending.logicOp = VK_LOGIC_OP_COPY;
    ColorBlending.attachmentCount = 1;
    ColorBlending.pAttachments = &ColorBlendAttachment;
    ColorBlending.blendConstants[0] = 0.0f;
    ColorBlending.blendConstants[1] = 0.0f;
    ColorBlending.blendConstants[2] = 0.0f;
    ColorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = 0;
    PipelineLayoutInfo.pSetLayouts = nullptr;
    PipelineLayoutInfo.pushConstantRangeCount = 0;
    PipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(Engine::Device, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }
    else
    {
        std::cout << "Successfully created pipeline layout \n";
    }

    VkGraphicsPipelineCreateInfo PipelineInfo{};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = ShaderStages;
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssembly;
    PipelineInfo.pViewportState = &ViewportState;
    PipelineInfo.pRasterizationState = &Rasterizer;
    PipelineInfo.pMultisampleState = &Multisampling;
    PipelineInfo.pDepthStencilState = nullptr;
    PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDynamicState = &DynamicState;
    PipelineInfo.layout = PipelineLayout;
    PipelineInfo.renderPass = RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(Engine::Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    else
    {
        std::cout << "Successfully created graphics pipeline \n";
    }

    vkDestroyShaderModule(Engine::Device, VertexShaderModule, nullptr);
    vkDestroyShaderModule(Engine::Device, FragmentShaderModule, nullptr);
}

void Engine::CreateFramebuffers()
{
    SwapChainFramebuffers.resize(SwapChainImageViews.size());

    for (size_t i = 0; i < SwapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            SwapChainImageViews[i]
        };

        VkFramebufferCreateInfo FramebufferInfo{};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = RenderPass;
        FramebufferInfo.attachmentCount = 1;
        FramebufferInfo.pAttachments = attachments;
        FramebufferInfo.width = SwapChainExtent.width;
        FramebufferInfo.height = SwapChainExtent.height;
        FramebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Engine::Device, &FramebufferInfo, nullptr, &SwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create frame buffer");
        }
        else
        {
            std::cout << "Successfully created frame buffer \n";
        }
    }
}

//static void Engine::FrameBufferResizeCallback(SDL_Window* Window)
//{
//    auto app = reinterpret_cast<Engine::Application*>(SDL_GetWindowData(Window, "SDL_Window"));
//    app->FramebufferResized = true;
//}

void Engine::CreateCommandBuffers()
{
    Engine::CommandBuffers.resize(Engine::MaxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)Engine::CommandBuffers.size();

    if (vkAllocateCommandBuffers(Engine::Device, &allocInfo, Engine::CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers");
    }
    else
    {
        std::cout << "Successfully created command buffer \n";
    }
}

void Engine::RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t ImageIndex)
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;
    BeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(CommandBuffer, &BeginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer");
    }
    else
    {
        //std::cout << "Successfully began recording command buffer \n";
    }

    VkRenderPassBeginInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = RenderPass;
    RenderPassInfo.framebuffer = SwapChainFramebuffers[ImageIndex];
    RenderPassInfo.renderArea.offset = { 0, 0 };
    RenderPassInfo.renderArea.extent = SwapChainExtent;

    VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };

    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

    VkViewport Viewport{};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = static_cast<float>(SwapChainExtent.width);
    Viewport.height = static_cast<float>(SwapChainExtent.height);
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor{};
    Scissor.offset = { 0, 0 };
    Scissor.extent = SwapChainExtent;
    vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

    vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(CommandBuffer);

    if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer");
    }
}

void Engine::CreateCommandPool()
{
    QueueFamilyIndices QueueFamilyIndices = FindQueueFamilies(Engine::PhysicalDevice);

    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    PoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();

    if (vkCreateCommandPool(Engine::Device, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool");
    }
    else
    {
        std::cout << "Successfully created command pool \n";
    }
}

void Engine::CreateSyncObjects()
{
    Engine::ImageAvailableSemaphores.resize(Engine::MaxFramesInFlight);
    Engine::RenderFinishedSemaphores.resize(Engine::MaxFramesInFlight);
    Engine::InFlightFences.resize(Engine::MaxFramesInFlight);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < Engine::MaxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(Engine::Device, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(Engine::Device, &SemaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(Engine::Device, &fenceInfo, nullptr, &InFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphores & fences");
        }
        else
        {
            std::cout << "Successfully created semaphores & fences \n";
        }
    }
}

void Engine::DrawFrame()
{
    vkWaitForFences(Engine::Device, 1, &Engine::InFlightFences[Engine::CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t ImageIndex;

    VkResult Result = vkAcquireNextImageKHR(Engine::Device, SwapChain, UINT64_MAX, Engine::ImageAvailableSemaphores[Engine::CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();

        return;
    }
    else if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    vkResetFences(Engine::Device, 1, &Engine::InFlightFences[Engine::CurrentFrame]);

    vkResetCommandBuffer(CommandBuffers[Engine::CurrentFrame], 0);

    RecordCommandBuffer(CommandBuffers[Engine::CurrentFrame], ImageIndex);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { ImageAvailableSemaphores[Engine::CurrentFrame] };

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = waitSemaphores;
    SubmitInfo.pWaitDstStageMask = waitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[Engine::CurrentFrame];

    VkSemaphore SignalSemaphores[] = { RenderFinishedSemaphores[Engine::CurrentFrame] };
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, InFlightFences[Engine::CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit the draw command buffer");
    }

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;

    VkSwapchainKHR SwapChains[] = { SwapChain };

    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || FramebufferResized)
    {
        std::cout << "Recreating swap chain from drawframe present \n";

        FramebufferResized = false;
        RecreateSwapChain();
    }
    else if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image");
    }

    CurrentFrame = (CurrentFrame + 1) % Engine::MaxFramesInFlight;
}

void Engine::CleanupSwapChain()
{
    for (auto Framebuffer : SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(Engine::Device, Framebuffer, nullptr);
    }

    for (auto ImageView : SwapChainImageViews)
    {
        vkDestroyImageView(Engine::Device, ImageView, nullptr);
    }

    vkDestroySwapchainKHR(Engine::Device, SwapChain, nullptr);
}

void Engine::Cleanup()
{
    Engine::CleanupSwapChain();

    vkDestroyPipeline(Engine::Device, GraphicsPipeline, nullptr);

    vkDestroyPipelineLayout(Engine::Device, PipelineLayout, nullptr);

    vkDestroyRenderPass(Engine::Device, RenderPass, nullptr);

    for (size_t i = 0; i < Engine::MaxFramesInFlight; i++)
    {
        vkDestroySemaphore(Engine::Device, Engine::RenderFinishedSemaphores[i], nullptr);

        vkDestroySemaphore(Engine::Device, Engine::ImageAvailableSemaphores[i], nullptr);

        vkDestroyFence(Engine::Device, Engine::InFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(Engine::Device, CommandPool, nullptr);

    vkDestroyDevice(Engine::Device, nullptr);

    if (EnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(Engine::Instance, DebugMessenger, nullptr);
    }
    DestroyDebugUtilsMessengerEXT(Engine::Instance, DebugMessenger, nullptr);

    vkDestroySurfaceKHR(Engine::Instance, Engine::Surface, nullptr);

    vkDestroyInstance(Engine::Instance, nullptr);

    SDL_DestroyWindow(Engine::Window);

    SDL_Quit();
}

static std::vector<char> Engine::ReadFile(const std::string& FileName)
{
    std::ifstream File(FileName, std::ios::ate | std::ios::binary);

    if (!File.is_open())
    {
        throw std::runtime_error("Failed to open file: " + FileName);
    }
    else
    {
        std::cout << "Successfully opened file: " + FileName + "\n";
    }

    size_t FileSize = (size_t)File.tellg();
    std::vector<char> Buffer(FileSize);

    File.seekg(0);
    File.read(Buffer.data(), FileSize);

    File.close();

    return Buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* PCallbackData, void* PUserData)
{
    std::cerr << "Validation layer: " << PCallbackData->pMessage << std::endl;

    return VK_FALSE;
}