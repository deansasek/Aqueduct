#include "../../Src/Common.h"

#include "Renderer.h"

#include "../../Src/Engine.h"

SDL_Window* Vulkan::Renderer::Window;
VkInstance Vulkan::Renderer::Instance;
VkSurfaceKHR Vulkan::Renderer::Surface;

VkPhysicalDevice Vulkan::Renderer::PhysicalDevice = VK_NULL_HANDLE;
VkDevice Vulkan::Renderer::Device;

VkQueue Vulkan::Renderer::GraphicsQueue;
VkQueue Vulkan::Renderer::PresentQueue;

VkPipelineLayout Vulkan::Renderer::PipelineLayout;
VkPipeline Vulkan::Renderer::GraphicsPipeline;

VkSwapchainKHR Vulkan::Renderer::SwapChain;
std::vector<VkImage> Vulkan::Renderer::SwapChainImages;
VkFormat Vulkan::Renderer::SwapChainImageFormat;
VkExtent2D Vulkan::Renderer::SwapChainExtent;
std::vector<VkImageView> Vulkan::Renderer::SwapChainImageViews;
std::vector<VkFramebuffer> Vulkan::Renderer::SwapChainFramebuffers;

VkRenderPass Vulkan::Renderer::RenderPass;

VkCommandPool Vulkan::Renderer::CommandPool;
std::vector<VkCommandBuffer> Vulkan::Renderer::CommandBuffers;

std::vector<VkSemaphore> Vulkan::Renderer::ImageAvailableSemaphores;
std::vector<VkSemaphore> Vulkan::Renderer::RenderFinishedSemaphores;
std::vector<VkFence> Vulkan::Renderer::InFlightFences;

VkDebugUtilsMessengerEXT Vulkan::Renderer::DebugMessenger;

bool Vulkan::Renderer::FramebufferResized = false;

uint32_t Vulkan::Renderer::CurrentFrame = 0;

void Vulkan::Renderer::Init()
{
    Vulkan::Renderer::CreateInstance();
    Vulkan::Renderer::InitDebugMessenger();
    Vulkan::Renderer::CreateSurface();
    Vulkan::Renderer::PickPhysicalDevice();
    Vulkan::Renderer::CreateLogicalDevice();
    Vulkan::Renderer::CreateSwapChain();
    Vulkan::Renderer::CreateImageViews();
    Vulkan::Renderer::CreateRenderPass();
    Vulkan::Renderer::CreateGraphicsPipeline();
    Vulkan::Renderer::CreateFramebuffers();
    Vulkan::Renderer::CreateCommandPool();
    Vulkan::Renderer::CreateCommandBuffers();
    Vulkan::Renderer::CreateSyncObjects();
}

void Vulkan::Renderer::CleanUpSwapChain()
{
    for (auto Framebuffer : Vulkan::Renderer::SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(Vulkan::Renderer::Device, Framebuffer, nullptr);
    }

    for (auto ImageView : Vulkan::Renderer::SwapChainImageViews)
    {
        vkDestroyImageView(Vulkan::Renderer::Device, ImageView, nullptr);
    }

    vkDestroySwapchainKHR(Vulkan::Renderer::Device, Vulkan::Renderer::SwapChain, nullptr);
}

void Vulkan::Renderer::CleanUp()
{
    Vulkan::Renderer::CleanUpSwapChain();

    vkDestroyPipeline(Vulkan::Renderer::Device, Vulkan::Renderer::GraphicsPipeline, nullptr);

    vkDestroyPipelineLayout(Vulkan::Renderer::Device, Vulkan::Renderer::PipelineLayout, nullptr);

    vkDestroyRenderPass(Vulkan::Renderer::Device, Vulkan::Renderer::RenderPass, nullptr);

    for (size_t i = 0; i < Vulkan::Renderer::MaxFramesInFlight; i++)
    {
        vkDestroySemaphore(Vulkan::Renderer::Device, Vulkan::Renderer::RenderFinishedSemaphores[i], nullptr);

        vkDestroySemaphore(Vulkan::Renderer::Device, Vulkan::Renderer::ImageAvailableSemaphores[i], nullptr);

        vkDestroyFence(Vulkan::Renderer::Device, Vulkan::Renderer::InFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(Vulkan::Renderer::Device, Vulkan::Renderer::CommandPool, nullptr);

    vkDestroyDevice(Vulkan::Renderer::Device, nullptr);

    if (Vulkan::Renderer::EnableValidationLayers)
    {
        Vulkan::Renderer::DestroyDebugUtilsMessengerEXT(Vulkan::Renderer::Instance, Vulkan::Renderer::DebugMessenger, nullptr);
    }
    Vulkan::Renderer::DestroyDebugUtilsMessengerEXT(Vulkan::Renderer::Instance, Vulkan::Renderer::DebugMessenger, nullptr);

    vkDestroySurfaceKHR(Vulkan::Renderer::Instance, Vulkan::Renderer::Surface, nullptr);

    vkDestroyInstance(Vulkan::Renderer::Instance, nullptr);

    SDL_DestroyWindow(Vulkan::Renderer::Window);

    SDL_Quit();
}

void Vulkan::Renderer::InitWindow()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);

    Vulkan::Renderer::Window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
    SDL_SetWindowData(Vulkan::Renderer::Window, "SDL_Window", &Vulkan::Renderer::Window);
}

void Vulkan::Renderer::CreateSurface()
{
    if (SDL_Vulkan_CreateSurface(Vulkan::Renderer::Window, Vulkan::Renderer::Instance, &Vulkan::Renderer::Surface) == SDL_FALSE)
    {
        throw std::runtime_error("VK > Failed to create window surface!");
    }
    else if (SDL_Vulkan_CreateSurface(Vulkan::Renderer::Window, Vulkan::Renderer::Instance, &Vulkan::Renderer::Surface) == SDL_TRUE)
    {
        std::cout << "VK > Successfully created window surface! \n";
    }
}

void Vulkan::Renderer::CreateInstance()
{
    if (Vulkan::Renderer::EnableValidationLayers && !Vulkan::Renderer::CheckValidationLayerSupport())
    {
        throw std::runtime_error("VK > Validation layers requested, but not available!");
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

    auto Extensions = Vulkan::Renderer::GetRequiredExtensions();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
    CreateInfo.ppEnabledExtensionNames = Extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

    if (Vulkan::Renderer::EnableValidationLayers)
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(Vulkan::Renderer::ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = Vulkan::Renderer::ValidationLayers.data();

        Vulkan::Renderer::PopulateDebugMessengerCreateInfo(DebugCreateInfo);
        CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
        CreateInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&CreateInfo, nullptr, &Vulkan::Renderer::Instance);

    if (vkCreateInstance(&CreateInfo, nullptr, &Vulkan::Renderer::Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create instance!");
    }
    else if (vkCreateInstance(&CreateInfo, nullptr, &Vulkan::Renderer::Instance) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created instance! \n";
    }
}

void Vulkan::Renderer::PickPhysicalDevice()
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Vulkan::Renderer::Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("No GPU detected with Vulkan support");
    }
    else
    {
        std::cout << "VK > Detected GPU with Vulkan support \n";
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Vulkan::Renderer::Instance, &DeviceCount, Devices.data());

    std::multimap<int, VkPhysicalDevice> Candidates;

    for (const auto& Device : Devices)
    {
        if (Vulkan::Renderer::IsDeviceSuitable(Device))
        {
            Vulkan::Renderer::PhysicalDevice = Device;

            std::cout << "VK > Successfully picked GPU with Vulkan support! \n";
        }
        else
        {
            int Score = Vulkan::Renderer::RateDeviceSuitability(Device);
            Candidates.insert(std::make_pair(Score, Device));

            if (Candidates.rbegin()->first > 0)
            {
                PhysicalDevice = Candidates.rbegin()->second;

                QueueFamilyIndices Indices = FindQueueFamilies(PhysicalDevice);

                if (Indices.IsComplete())
                {
                    std::cout << "VK > Successfully picked GPU with Vulkan support! \n";
                    Vulkan::Renderer::PhysicalDevice = PhysicalDevice;
                }
            }
            else
            {
                throw std::runtime_error("VK > Failed to locate GPU with Vulkan support!");
            }

            if (PhysicalDevice == VK_NULL_HANDLE)
            {
                throw std::runtime_error("VK > Failed to locate GPU with Vulkan support!");
            }
        }
    }
}

bool Vulkan::Renderer::IsDeviceSuitable(VkPhysicalDevice Device)
{
    Vulkan::Renderer::QueueFamilyIndices Indices = Vulkan::Renderer::FindQueueFamilies(Device);

    return Indices.IsComplete();
}

int Vulkan::Renderer::RateDeviceSuitability(VkPhysicalDevice Device)
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

    bool ExtensionsSupported = Vulkan::Renderer::CheckDeviceExtensionSupport(Device);

    bool SwapChainAdequate = false;

    if (ExtensionsSupported)
    {
        Vulkan::Renderer::SwapChainSupportDetails SwapChainSupport = Vulkan::Renderer::QuerySwapChainSupport(Device);
        SwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentModes.empty();
    }

    return Score && ExtensionsSupported && SwapChainAdequate;
}

bool Vulkan::Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice Device)
{
    uint32_t ExtensionCount;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr);

    std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, AvailableExtensions.data());

    std::set<std::string> RequiredExtensions(Vulkan::Renderer::DeviceExtensions.begin(), Vulkan::Renderer::DeviceExtensions.end());

    for (const auto& Extension : AvailableExtensions)
    {
        RequiredExtensions.erase(Extension.extensionName);
    }

    return RequiredExtensions.empty();
}

void Vulkan::Renderer::CreateLogicalDevice()
{
    QueueFamilyIndices Indices = FindQueueFamilies(Vulkan::Renderer::PhysicalDevice);

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

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Vulkan::Renderer::DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = Vulkan::Renderer::DeviceExtensions.data();

    // add validation layers if enabled
    if (Vulkan::Renderer::EnableValidationLayers)
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(Vulkan::Renderer::PhysicalDevice, &CreateInfo, nullptr, &Vulkan::Renderer::Device) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create logical device!");
    }
    else if (vkCreateDevice(Vulkan::Renderer::PhysicalDevice, &CreateInfo, nullptr, &Vulkan::Renderer::Device) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created logical device! \n";
    }

    vkGetDeviceQueue(Vulkan::Renderer::Device, Indices.GraphicsFamily.value(), 0, &GraphicsQueue);
    vkGetDeviceQueue(Vulkan::Renderer::Device, Indices.PresentFamily.value(), 0, &PresentQueue);
}

bool Vulkan::Renderer::CheckValidationLayerSupport()
{
    uint32_t LayerCount;
    vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

    std::vector<VkLayerProperties> AvailableLayers(LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

    for (const char* LayerName : Vulkan::Renderer::ValidationLayers)
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

std::vector<const char*> Vulkan::Renderer::GetRequiredExtensions()
{
    uint32_t SDL2ExtensionCount;

    const char** SDL2ExtensionNames;

    SDL_Vulkan_GetInstanceExtensions(Vulkan::Renderer::Window, &SDL2ExtensionCount, nullptr);

    SDL2ExtensionNames = new const char* [SDL2ExtensionCount];

    SDL_Vulkan_GetInstanceExtensions(Vulkan::Renderer::Window, &SDL2ExtensionCount, SDL2ExtensionNames);

    std::vector<const char*> SLD2Extensions(SDL2ExtensionNames, SDL2ExtensionNames + SDL2ExtensionCount);

    if (Vulkan::Renderer::EnableValidationLayers)
    {
        SLD2Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return SLD2Extensions;
}

Vulkan::Renderer::QueueFamilyIndices Vulkan::Renderer::FindQueueFamilies(VkPhysicalDevice Device)
{
    Vulkan::Renderer::QueueFamilyIndices Indices;

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies.data());

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
        vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Vulkan::Renderer::Surface, &PresentSupport);

        if (PresentSupport)
        {
            Indices.PresentFamily = i;
        }

        if (Indices.IsComplete())
        {
            break;
        }

        i++;
    }

    return Indices;
}

void Vulkan::Renderer::CreateSwapChain()
{
    Vulkan::Renderer::SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(Vulkan::Renderer::PhysicalDevice);

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
    CreateInfo.surface = Vulkan::Renderer::Surface;
    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Vulkan::Renderer::QueueFamilyIndices Indices = Vulkan::Renderer::FindQueueFamilies(Vulkan::Renderer::PhysicalDevice);
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

    if (vkCreateSwapchainKHR(Vulkan::Renderer::Device, &CreateInfo, nullptr, &SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create swap chain!");
    }
    else
    {
        std::cout << "VK > Successfully created swap chain \n";
    }

    vkGetSwapchainImagesKHR(Vulkan::Renderer::Device, SwapChain, &ImageCount, nullptr);
    Vulkan::Renderer::SwapChainImages.resize(ImageCount);
    vkGetSwapchainImagesKHR(Vulkan::Renderer::Device, SwapChain, &ImageCount, Vulkan::Renderer::SwapChainImages.data());

    Vulkan::Renderer::SwapChainImageFormat = SurfaceFormat.format;
    Vulkan::Renderer::SwapChainExtent = Extent;
}

Vulkan::Renderer::SwapChainSupportDetails Vulkan::Renderer::QuerySwapChainSupport(VkPhysicalDevice Device)
{
    Vulkan::Renderer::SwapChainSupportDetails Details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Vulkan::Renderer::Surface, &Details.Capabilities);

    uint32_t FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Vulkan::Renderer::Surface, &FormatCount, nullptr);

    if (FormatCount != 0)
    {
        Details.Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Vulkan::Renderer::Surface, &FormatCount, Details.Formats.data());
    }

    // gets supported present modes
    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Vulkan::Renderer::Surface, &PresentModeCount, nullptr);

    if (FormatCount != 0)
    {
        Details.PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Vulkan::Renderer::Surface, &PresentModeCount, Details.PresentModes.data());
    }

    return Details;
}

VkSurfaceFormatKHR Vulkan::Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
{
    for (const auto& AvailableFormat : AvailableFormats)
    {
        if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return AvailableFormat;
        }
    }

    return AvailableFormats[0];
}

VkPresentModeKHR Vulkan::Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes)
{
    for (const auto& AvailablePresentMode : AvailablePresentModes)
    {
        if (AvailablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            return AvailablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
{
    if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return Capabilities.currentExtent;
    }
    else
    {
        int Width, Height;

        SDL_Vulkan_GetDrawableSize(Vulkan::Renderer::Window, &Width, &Height);

        VkExtent2D ActualExtent = {
            static_cast<uint32_t>(Width),
            static_cast<uint32_t>(Height)
        };

        ActualExtent.width = std::clamp(ActualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        ActualExtent.height = std::clamp(ActualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

        return ActualExtent;
    }
}

void Vulkan::Renderer::RecreateSwapChain()
{
    vkDeviceWaitIdle(Vulkan::Renderer::Device);

    Vulkan::Renderer::CleanUpSwapChain();

    Vulkan::Renderer::CreateSwapChain();
    Vulkan::Renderer::CreateImageViews();
    Vulkan::Renderer::CreateFramebuffers();
}

void Vulkan::Renderer::CreateImageViews()
{
    Vulkan::Renderer::SwapChainImageViews.resize(Vulkan::Renderer::SwapChainImages.size());

    for (size_t i = 0; i < Vulkan::Renderer::SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        CreateInfo.image = Vulkan::Renderer::SwapChainImages[i];
        CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        CreateInfo.format = Vulkan::Renderer::SwapChainImageFormat;

        CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        CreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        CreateInfo.subresourceRange.baseMipLevel = 0;
        CreateInfo.subresourceRange.levelCount = 1;
        CreateInfo.subresourceRange.baseArrayLayer = 0;
        CreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Vulkan::Renderer::SwapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("VK > Failed to create image views!");
        }
        else
        {
            std::cout << "VK > Successfully created image views! \n";
        }
    }
}

void Vulkan::Renderer::CreateFramebuffers()
{
    Vulkan::Renderer::SwapChainFramebuffers.resize(Vulkan::Renderer::SwapChainImageViews.size());

    for (size_t i = 0; i < Vulkan::Renderer::SwapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            Vulkan::Renderer::SwapChainImageViews[i]
        };

        VkFramebufferCreateInfo FramebufferInfo{};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = Vulkan::Renderer::RenderPass;
        FramebufferInfo.attachmentCount = 1;
        FramebufferInfo.pAttachments = attachments;
        FramebufferInfo.width = SwapChainExtent.width;
        FramebufferInfo.height = SwapChainExtent.height;
        FramebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Vulkan::Renderer::Device, &FramebufferInfo, nullptr, &Vulkan::Renderer::SwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("VK > Failed to create frame buffer!");
        }
        else
        {
            std::cout << "VK > Successfully created frame buffer! \n";
        }
    }
}

//static void Engine::FrameBufferResizeCallback(SDL_Window* Window)
//{
//    auto app = reinterpret_cast<Engine::Application*>(SDL_GetWindowData(Window, "SDL_Window"));
//    app->FramebufferResized = true;
//}

void Vulkan::Renderer::CreateGraphicsPipeline()
{
    auto VertexShaderCode = Vulkan::Renderer::ReadFile("Shaders/Vert.spv");
    auto FragmentShaderCode = Vulkan::Renderer::ReadFile("Shaders/Frag.spv");

    VkShaderModule VertexShaderModule = Vulkan::Renderer::CreateShaderModule(VertexShaderCode);
    VkShaderModule FragmentShaderModule = Vulkan::Renderer::CreateShaderModule(FragmentShaderCode);

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
    Viewport.width = (float)SwapChainExtent.width;
    Viewport.height = (float)SwapChainExtent.height;
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

    if (vkCreatePipelineLayout(Vulkan::Renderer::Device, &PipelineLayoutInfo, nullptr, &Vulkan::Renderer::PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create pipeline layout!");
    }
    else
    {
        std::cout << "VK > Successfully created pipeline layout! \n";
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

    if (vkCreateGraphicsPipelines(Vulkan::Renderer::Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &Vulkan::Renderer::GraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create graphics pipeline!");
    }
    else
    {
        std::cout << "VK > Successfully created graphics pipeline! \n";
    }

    vkDestroyShaderModule(Vulkan::Renderer::Device, VertexShaderModule, nullptr);
    vkDestroyShaderModule(Vulkan::Renderer::Device, FragmentShaderModule, nullptr);
}

VkShaderModule Vulkan::Renderer::CreateShaderModule(const std::vector<char>& Code)
{
    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = Code.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

    VkShaderModule ShaderModule;

    if (vkCreateShaderModule(Vulkan::Renderer::Device, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create shader module!");
    }
    else
    {
        std::cout << "VK > Successfully created shader module! \n";
    }

    return ShaderModule;
}

void Vulkan::Renderer::CreateRenderPass()
{
    VkAttachmentDescription ColorAttachment{};
    ColorAttachment.format = Vulkan::Renderer::SwapChainImageFormat;
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

    if (vkCreateRenderPass(Vulkan::Renderer::Device, &RenderPassInfo, nullptr, &Vulkan::Renderer::RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("VL > Failed to create render pass!");
    }
    else
    {
        std::cout << "VK > Successfully created render pass! \n";
    }
}

void Vulkan::Renderer::CreateSyncObjects()
{
    Vulkan::Renderer::ImageAvailableSemaphores.resize(Vulkan::Renderer::MaxFramesInFlight);
    Vulkan::Renderer::RenderFinishedSemaphores.resize(Vulkan::Renderer::MaxFramesInFlight);
    Vulkan::Renderer::InFlightFences.resize(Vulkan::Renderer::MaxFramesInFlight);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < Vulkan::Renderer::MaxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(Vulkan::Renderer::Device, &SemaphoreInfo, nullptr, &Vulkan::Renderer::ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(Vulkan::Renderer::Device, &SemaphoreInfo, nullptr, &Vulkan::Renderer::RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(Vulkan::Renderer::Device, &fenceInfo, nullptr, &Vulkan::Renderer::InFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("VK > Failed to create semaphores & fences!");
        }
        else
        {
            std::cout << "VK > Successfully created semaphores & fences! \n";
        }
    }
}

void Vulkan::Renderer::DrawFrame()
{
    vkWaitForFences(Vulkan::Renderer::Device, 1, &Vulkan::Renderer::InFlightFences[Vulkan::Renderer::CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t ImageIndex;

    VkResult Result = vkAcquireNextImageKHR(Vulkan::Renderer::Device, Vulkan::Renderer::SwapChain, UINT64_MAX, Vulkan::Renderer::ImageAvailableSemaphores[Vulkan::Renderer::CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Vulkan::Renderer::RecreateSwapChain();

        return;
    }
    else if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to acquire swap chain image!");
    }

    vkResetFences(Vulkan::Renderer::Device, 1, &Vulkan::Renderer::InFlightFences[Vulkan::Renderer::CurrentFrame]);

    vkResetCommandBuffer(Vulkan::Renderer::CommandBuffers[Vulkan::Renderer::CurrentFrame], 0);

    Vulkan::Renderer::RecordCommandBuffer(Vulkan::Renderer::CommandBuffers[Vulkan::Renderer::CurrentFrame], ImageIndex);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { Vulkan::Renderer::ImageAvailableSemaphores[Vulkan::Renderer::CurrentFrame] };

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = waitSemaphores;
    SubmitInfo.pWaitDstStageMask = waitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Vulkan::Renderer::CommandBuffers[Vulkan::Renderer::CurrentFrame];

    VkSemaphore SignalSemaphores[] = { Vulkan::Renderer::RenderFinishedSemaphores[Vulkan::Renderer::CurrentFrame] };
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    if (vkQueueSubmit(Vulkan::Renderer::GraphicsQueue, 1, &SubmitInfo, Vulkan::Renderer::InFlightFences[Vulkan::Renderer::CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to submit the draw command buffer!");
    }

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;

    VkSwapchainKHR SwapChains[] = { Vulkan::Renderer::SwapChain };

    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    Result = vkQueuePresentKHR(Vulkan::Renderer::PresentQueue, &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || Vulkan::Renderer::FramebufferResized)
    {
        std::cout << "VK > Recreating swap chain from drawframe present! \n";

        Vulkan::Renderer::FramebufferResized = false;
        Vulkan::Renderer::RecreateSwapChain();
    }
    else if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to present swap chain image!");
    }

    Vulkan::Renderer::CurrentFrame = (Vulkan::Renderer::CurrentFrame + 1) % Vulkan::Renderer::MaxFramesInFlight;
}

void Vulkan::Renderer::CreateCommandBuffers()
{
    Vulkan::Renderer::CommandBuffers.resize(Vulkan::Renderer::MaxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = Vulkan::Renderer::CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)Vulkan::Renderer::CommandBuffers.size();

    if (vkAllocateCommandBuffers(Vulkan::Renderer::Device, &allocInfo, Vulkan::Renderer::CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to allocate command buffers!");
    }
    else
    {
        std::cout << "VK > Successfully created command buffer! \n";
    }
}

void Vulkan::Renderer::RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t ImageIndex)
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;
    BeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(CommandBuffer, &BeginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = Vulkan::Renderer::RenderPass;
    RenderPassInfo.framebuffer = Vulkan::Renderer::SwapChainFramebuffers[ImageIndex];
    RenderPassInfo.renderArea.offset = { 0, 0 };
    RenderPassInfo.renderArea.extent = Vulkan::Renderer::SwapChainExtent;

    VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };

    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan::Renderer::GraphicsPipeline);

    VkViewport Viewport{};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = static_cast<float>(Vulkan::Renderer::SwapChainExtent.width);
    Viewport.height = static_cast<float>(Vulkan::Renderer::SwapChainExtent.height);
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor{};
    Scissor.offset = { 0, 0 };
    Scissor.extent = Vulkan::Renderer::SwapChainExtent;
    vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

    vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(CommandBuffer);

    if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to record command buffer!");
    }
}

void Vulkan::Renderer::CreateCommandPool()
{
    Vulkan::Renderer::QueueFamilyIndices QueueFamilyIndices = Vulkan::Renderer::FindQueueFamilies(Vulkan::Renderer::PhysicalDevice);

    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    PoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();

    if (vkCreateCommandPool(Vulkan::Renderer::Device, &PoolInfo, nullptr, &Vulkan::Renderer::CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create command pool!");
    }
    else
    {
        std::cout << "VK > Successfully created command pool! \n";
    }
}

void Vulkan::Renderer::InitDebugMessenger()
{
    if (!Vulkan::Renderer::EnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
    Vulkan::Renderer::PopulateDebugMessengerCreateInfo(CreateInfo);

    if (Vulkan::Renderer::CreateDebugUtilsMessengerEXT(Vulkan::Renderer::Instance, &CreateInfo, nullptr, &Vulkan::Renderer::DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create debug messenger!");
    }
    else if (Vulkan::Renderer::CreateDebugUtilsMessengerEXT(Vulkan::Renderer::Instance, &CreateInfo, nullptr, &Vulkan::Renderer::DebugMessenger) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created debug messenger! \n";
    }
}

VkResult Vulkan::Renderer::CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* PAllocator, VkDebugUtilsMessengerEXT* PDebugMessenger)
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

void Vulkan::Renderer::DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* PAllocator)
{
    auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");

    if (Func != nullptr)
    {
        Func(Instance, DebugMessenger, PAllocator);
    }
}

void Vulkan::Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo)
{
    CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    CreateInfo.pfnUserCallback = Vulkan::Renderer::DebugCallback;
    CreateInfo.pUserData = nullptr;
}

/*
    FUTURE: Get this into the header file
*/

static VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::Renderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* PCallbackData, void* PUserData)
{
    std::cerr << "VK > Validation layer: " << PCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static std::vector<char> Vulkan::Renderer::ReadFile(const std::string& FileName)
{
    std::ifstream File(FileName, std::ios::ate | std::ios::binary);

    if (!File.is_open())
    {
        throw std::runtime_error("FS > Failed to open file: " + FileName);
    }
    else
    {
        std::cout << "FS > Successfully opened file: " + FileName + "\n";
    }

    size_t FileSize = (size_t)File.tellg();
    std::vector<char> Buffer(FileSize);

    File.seekg(0);
    File.read(Buffer.data(), FileSize);

    File.close();

    return Buffer;
}