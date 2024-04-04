#include "../../Src/Common.h"

#include "Renderer.h"

#include "../../Src/Engine.h"

#include "../../FileSystem/FileSystem.h"

#include "../../Camera.h"
#include "../../Model.h"
#include "../../GameObject.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

SDL_Window* Vulkan::Renderer::Window;
VkInstance Vulkan::Renderer::Instance;
VkSurfaceKHR Vulkan::Renderer::Surface;

std::multimap<int, VkPhysicalDevice> Vulkan::Renderer::Candidates;
VkPhysicalDevice Vulkan::Renderer::PhysicalDevice = VK_NULL_HANDLE;
VkDevice Vulkan::Renderer::Device;

VkQueue Vulkan::Renderer::GraphicsQueue;
VkQueue Vulkan::Renderer::PresentQueue;

VkDescriptorSetLayout Vulkan::Renderer::DescriptorSetLayout;
VkPipelineLayout Vulkan::Renderer::PipelineLayout;
VkPipeline Vulkan::Renderer::GraphicsPipeline;
VkPipeline Vulkan::Renderer::WireframePipeline;

std::vector<VkShaderModule> Vulkan::Renderer::ShaderModules;

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

//VkBuffer Vulkan::Renderer::VertexBuffer;
//VkDeviceMemory Vulkan::Renderer::VertexBufferMemory;

//VkBuffer Vulkan::Renderer::IndexBuffer;
//VkDeviceMemory Vulkan::Renderer::IndexBufferMemory;

std::vector<VkBuffer> Vulkan::Renderer::UniformBuffers;
std::vector<VkDeviceMemory> Vulkan::Renderer::UniformBuffersMemory;
std::vector<void*> Vulkan::Renderer::UniformBuffersMapped;

VkDescriptorPool Vulkan::Renderer::DescriptorPool;
std::vector<VkDescriptorSet> Vulkan::Renderer::DescriptorSets;

VkImage Vulkan::Renderer::TextureImage;
VkDeviceMemory Vulkan::Renderer::TextureImageMemory;
VkImageView Vulkan::Renderer::TextureImageView;
VkSampler Vulkan::Renderer::TextureSampler;

VkImage Vulkan::Renderer::DepthImage;
VkDeviceMemory Vulkan::Renderer::DepthImageMemory;
VkImageView Vulkan::Renderer::DepthImageView;

//std::vector<Vulkan::Renderer::Vertex> Vulkan::Renderer::Vertices;
//std::vector<uint32_t> Vulkan::Renderer::Indices;

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
    Vulkan::Renderer::CreateDescriptorSetLayout();
    Vulkan::Renderer::CreatePipelines();
    Vulkan::Renderer::CreateCommandPool();
    Vulkan::Renderer::CreateDepthResources();
    Vulkan::Renderer::CreateFramebuffers();
    Vulkan::Renderer::CreateTextureImage();
    Vulkan::Renderer::CreateTextureImageView();
    Vulkan::Renderer::CreateTextureSampler();
    Engine::GameObject::CreateGameObjects();
    Vulkan::Renderer::CreateUniformBuffers();
    Vulkan::Renderer::CreateDescriptorPool();
    Vulkan::Renderer::CreateDescriptorSets();
    Vulkan::Renderer::CreateCommandBuffers();
    Vulkan::Renderer::CreateSyncObjects();
}

void Vulkan::Renderer::CleanUpSwapChain()
{
    vkDestroyImageView(Vulkan::Renderer::Device, Vulkan::Renderer::DepthImageView, nullptr);
    vkDestroyImage(Vulkan::Renderer::Device, Vulkan::Renderer::DepthImage, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, Vulkan::Renderer::DepthImageMemory, nullptr);

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

    vkDestroySampler(Vulkan::Renderer::Device, Vulkan::Renderer::TextureSampler, nullptr);
    vkDestroyImageView(Vulkan::Renderer::Device, Vulkan::Renderer::TextureImageView, nullptr);

    vkDestroyImage(Vulkan::Renderer::Device, Vulkan::Renderer::TextureImage, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, Vulkan::Renderer::TextureImageMemory, nullptr);

    //vkDestroyBuffer(Vulkan::Renderer::Device, Vulkan::Renderer::IndexBuffer, nullptr);
    //vkFreeMemory(Vulkan::Renderer::Device, Vulkan::Renderer::IndexBufferMemory, nullptr);

    //vkDestroyBuffer(Vulkan::Renderer::Device, Vulkan::Renderer::VertexBuffer, nullptr);
    //vkFreeMemory(Vulkan::Renderer::Device, Vulkan::Renderer::VertexBufferMemory, nullptr);

    for (size_t i = 0; i < Vulkan::Renderer::MaxFramesInFlight; i++)
    {
        vkDestroyBuffer(Vulkan::Renderer::Device, Vulkan::Renderer::UniformBuffers[i], nullptr);
        vkFreeMemory(Vulkan::Renderer::Device, Vulkan::Renderer::UniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(Vulkan::Renderer::Device, Vulkan::Renderer::DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(Vulkan::Renderer::Device, Vulkan::Renderer::DescriptorSetLayout, nullptr);

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

    Vulkan::Renderer::Window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
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
            Vulkan::Renderer::Candidates.insert(std::make_pair(Score, Device));

            if (Vulkan::Renderer::Candidates.rbegin()->first > 0)
            {
                PhysicalDevice = Vulkan::Renderer::Candidates.rbegin()->second;

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

    return Score && ExtensionsSupported && SwapChainAdequate && DeviceFeatures.samplerAnisotropy;
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

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());

    CreateInfo.pEnabledFeatures = &DeviceFeatures;

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Vulkan::Renderer::DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = Vulkan::Renderer::DeviceExtensions.data();

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

    for (const auto& QueueFamily : QueueFamilies)
    {
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
    Vulkan::Renderer::CreateDepthResources();
    Vulkan::Renderer::CreateFramebuffers();
}

void Vulkan::Renderer::CreateImageViews()
{
    Vulkan::Renderer::SwapChainImageViews.resize(Vulkan::Renderer::SwapChainImages.size());

    for (size_t i = 0; i < Vulkan::Renderer::SwapChainImages.size(); i++)
    {
        Vulkan::Renderer::SwapChainImageViews[i] = Vulkan::Renderer::CreateImageView(Vulkan::Renderer::SwapChainImages[i], Vulkan::Renderer::SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Vulkan::Renderer::CreateFramebuffers()
{
    Vulkan::Renderer::SwapChainFramebuffers.resize(Vulkan::Renderer::SwapChainImageViews.size());

    for (size_t i = 0; i < Vulkan::Renderer::SwapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> Attachments = {
            Vulkan::Renderer::SwapChainImageViews[i],
            Vulkan::Renderer::DepthImageView
        };

        VkFramebufferCreateInfo FramebufferInfo{};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = Vulkan::Renderer::RenderPass;
        FramebufferInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
        FramebufferInfo.pAttachments = Attachments.data();
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

void Vulkan::Renderer::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding UBOLayoutBinding{};
    UBOLayoutBinding.binding = 0;
    UBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    UBOLayoutBinding.descriptorCount = 1;
    UBOLayoutBinding.pImmutableSamplers = nullptr;
    UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding SamplerLayoutBinding{};
    SamplerLayoutBinding.binding = 1;
    SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerLayoutBinding.descriptorCount = 1;
    SamplerLayoutBinding.pImmutableSamplers = nullptr;
    SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> Bindings = { UBOLayoutBinding, SamplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo LayoutInfo{};
    LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutInfo.bindingCount = Bindings.size();
    LayoutInfo.pBindings = Bindings.data();

    if (vkCreateDescriptorSetLayout(Vulkan::Renderer::Device, &LayoutInfo, nullptr, &Vulkan::Renderer::DescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create descriptor set layout!");
    }
    else if (vkCreateDescriptorSetLayout(Vulkan::Renderer::Device, &LayoutInfo, nullptr, &Vulkan::Renderer::DescriptorSetLayout) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created descriptor set layout! \n";
    }
}

VkPipelineShaderStageCreateInfo Vulkan::Renderer::LoadShader(std::string FileName, VkShaderStageFlagBits Stage)
{
    VkPipelineShaderStageCreateInfo ShaderStage{};
    ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ShaderStage.stage = Stage;
    ShaderStage.module = Vulkan::Renderer::CreateShaderModule(FileSystem::ReadFile(FileName));
    ShaderStage.pName = "main";
    ShaderStage.pSpecializationInfo = nullptr;

    return ShaderStage;
}

void Vulkan::Renderer::CreatePipelines()
{
    auto BaseVertexShader = FileSystem::ReadFile("Shaders/BaseVertexShader.spv");
    auto BaseFragmentShader = FileSystem::ReadFile("Shaders/BaseFragmentShader.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> ShaderStages;

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

    auto BindingDescription = Vulkan::Renderer::Vertex::GetBindingDescription();
    auto AttributeDescriptions = Vulkan::Renderer::Vertex::GetAttributeDescriptions();

    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescriptions.size());

    VertexInputInfo.pVertexBindingDescriptions = &BindingDescription;
    VertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions.data();

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
    Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

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

    VkPipelineDepthStencilStateCreateInfo DepthStencil{};
    DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencil.depthTestEnable = VK_TRUE;
    DepthStencil.depthWriteEnable = VK_TRUE;
    DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.minDepthBounds = 0.0f;
    DepthStencil.maxDepthBounds = 1.0f;
    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};
    DepthStencil.back = {};

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

    VkPushConstantRange PushConstantRange;
    PushConstantRange.offset = 0;
    PushConstantRange.size = sizeof(glm::mat4);
    PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    
    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = 1;
    PipelineLayoutInfo.pSetLayouts = &Vulkan::Renderer::DescriptorSetLayout;
    PipelineLayoutInfo.pushConstantRangeCount = 1;
    PipelineLayoutInfo.pPushConstantRanges = &PushConstantRange;

    if (vkCreatePipelineLayout(Vulkan::Renderer::Device, &PipelineLayoutInfo, nullptr, &Vulkan::Renderer::PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create pipeline layout!");
    }
    else if (vkCreatePipelineLayout(Vulkan::Renderer::Device, &PipelineLayoutInfo, nullptr, &Vulkan::Renderer::PipelineLayout) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created pipeline layout! \n";
    }

    VkGraphicsPipelineCreateInfo PipelineInfo{};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = static_cast<uint32_t>(ShaderStages.size());
    PipelineInfo.pStages = ShaderStages.data();
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssembly;
    PipelineInfo.pViewportState = &ViewportState;
    PipelineInfo.pRasterizationState = &Rasterizer;
    PipelineInfo.pMultisampleState = &Multisampling;
    PipelineInfo.pDepthStencilState = &DepthStencil;
    PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDynamicState = &DynamicState;
    PipelineInfo.layout = PipelineLayout;
    PipelineInfo.renderPass = RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    /*
        Normal shader stages + pipeline
    */

    ShaderStages[0] = Vulkan::Renderer::LoadShader("Shaders/BaseVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
    ShaderStages[1] = Vulkan::Renderer::LoadShader("Shaders/BaseFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    if (vkCreateGraphicsPipelines(Vulkan::Renderer::Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &Vulkan::Renderer::Pipelines.Normal) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create normal pipeline!");
    }
    else
    {
        std::cout << "VK > Successfully created normal pipeline! \n";
    }

    /*
        Wireframe shader stages + pipeline
    */

   // Rasterizer.polygonMode = VK_POLYGON_MODE_LINE;

    ShaderStages[0] = Vulkan::Renderer::LoadShader("Shaders/LightingVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
    ShaderStages[1] = Vulkan::Renderer::LoadShader("Shaders/LightingFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    if (vkCreateGraphicsPipelines(Vulkan::Renderer::Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &Vulkan::Renderer::Pipelines.WireFrame) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create wireframe pipeline!");
    }
    else
    {
        std::cout << "VK > Successfully created wireframe pipeline! \n";
    }

    for (auto& ShaderModule : Vulkan::Renderer::ShaderModules)
    {
        vkDestroyShaderModule(Vulkan::Renderer::Device, ShaderModule, nullptr);
    }
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

    Vulkan::Renderer::ShaderModules.push_back(ShaderModule);

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

    VkAttachmentDescription DepthAttachment{};
    DepthAttachment.format = Vulkan::Renderer::FindDepthFormat();
    DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthAttachmentRef{};
    DepthAttachmentRef.attachment = 1;
    DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubPass{};
    SubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubPass.colorAttachmentCount = 1;
    SubPass.pColorAttachments = &ColorAttachmentRef;
    SubPass.pDepthStencilAttachment = &DepthAttachmentRef;

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> Attachments = { ColorAttachment, DepthAttachment };
    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
    RenderPassInfo.pAttachments = Attachments.data();
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

    Vulkan::Renderer::UpdateUniformBuffer(Vulkan::Renderer::CurrentFrame);

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

    std::array<VkClearValue, 2> ClearValues{};
    ClearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    ClearValues[1].depthStencil = { 1.0f, 0 };

    RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
    RenderPassInfo.pClearValues = ClearValues.data();

    vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan::Renderer::PipelineLayout, 0, 1, &Vulkan::Renderer::DescriptorSets[Vulkan::Renderer::CurrentFrame], 0, nullptr);

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

    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan::Renderer::Pipelines.Normal);

    /*
        Model magic
    */

    /*
        This can probably be done better
    */


    Engine::GameObject::RenderGameObjects(CommandBuffer);

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

void Vulkan::Renderer::CreateImage(uint32_t Width, uint32_t Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImage& Image, VkDeviceMemory& ImageMemory)
{
    VkImageCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    CreateInfo.imageType = VK_IMAGE_TYPE_2D;
    CreateInfo.extent.width = static_cast<uint32_t>(Width);
    CreateInfo.extent.height = static_cast<uint32_t>(Height);
    CreateInfo.extent.depth = 1;
    CreateInfo.mipLevels = 1;
    CreateInfo.arrayLayers = 1;
    CreateInfo.format = Format;
    CreateInfo.tiling = Tiling;
    CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    CreateInfo.usage = Usage;
    CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    CreateInfo.flags = 0;

    if (vkCreateImage(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Image) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create texture image!");
    }
    else if (vkCreateImage(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Image) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created texture image!" << std::endl;
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetImageMemoryRequirements(Vulkan::Renderer::Device, Image, &MemoryRequirements);

    VkMemoryAllocateInfo AllocateInfo{};
    AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocateInfo.allocationSize = MemoryRequirements.size;
    AllocateInfo.memoryTypeIndex = Vulkan::Renderer::FindMemoryType(MemoryRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(Vulkan::Renderer::Device, &AllocateInfo, nullptr, &ImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to allocate texture image memory!");
    }
    else
    {
        std::cout << "VK > Successfully allocated texture image memory!" << std::endl;
    }

    vkBindImageMemory(Vulkan::Renderer::Device, Image, ImageMemory, 0);
}

void Vulkan::Renderer::TransitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout)
{
    VkCommandBuffer CommandBuffer = Vulkan::Renderer::BeginSingleTimeCommands();

    VkImageMemoryBarrier Barrier{};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    Barrier.oldLayout = OldLayout;
    Barrier.newLayout = NewLayout;

    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    Barrier.image = Image;

    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags SourceStage;
    VkPipelineStageFlags DestinationStage;

    if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (Vulkan::Renderer::HasStencilComponent(Format))
        {
            Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("VK > Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(CommandBuffer, SourceStage, DestinationStage, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

    Vulkan::Renderer::EndSingleTimeCommands(CommandBuffer);
}

VkFormat Vulkan::Renderer::FindSupportedFormats(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
    for (VkFormat Format : Candidates)
    {
        VkFormatProperties Properties;
        vkGetPhysicalDeviceFormatProperties(Vulkan::Renderer::PhysicalDevice, Format, &Properties);

        if (Tiling == VK_IMAGE_TILING_LINEAR && (Properties.linearTilingFeatures & Features) == Features)
        {
            std::cout << "VK > Successfully found supported format!" << std::endl;

            return Format;
        }
        else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (Properties.optimalTilingFeatures & Features) == Features)
        {
            std::cout << "VK > Successfully found supported format!" << std::endl;

            return Format;
        }
    }

    throw std::runtime_error("VK > Failed to find supported format!");
}

bool Vulkan::Renderer::HasStencilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat Vulkan::Renderer::FindDepthFormat()
{
    return Vulkan::Renderer::FindSupportedFormats(
        { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void Vulkan::Renderer::CreateDepthResources()
{
    VkFormat DepthFormat = Vulkan::Renderer::FindDepthFormat();

    Vulkan::Renderer::CreateImage(Vulkan::Renderer::SwapChainExtent.width, Vulkan::Renderer::SwapChainExtent.height, DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Vulkan::Renderer::DepthImage, Vulkan::Renderer::DepthImageMemory);
    Vulkan::Renderer::DepthImageView = Vulkan::Renderer::CreateImageView(Vulkan::Renderer::DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    Vulkan::Renderer::TransitionImageLayout(Vulkan::Renderer::DepthImage, DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);


}

void Vulkan::Renderer::CreateTextureImage()
{
    int TextureWidth, TextureHeight, TextureChannels;

    stbi_uc* Pixels = stbi_load(Engine::Model::TexturePath.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, STBI_rgb_alpha);

    VkDeviceSize ImageSize = TextureWidth * TextureHeight * 4;

    if (!Pixels)
    {
        throw std::runtime_error("VK > Failed to load texture image!");
    }
    else
    {
        std::cout << "VK > Successfully loaded texture image! \n";
    }

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;
    Vulkan::Renderer::CreateBuffer(ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* Data;
    vkMapMemory(Vulkan::Renderer::Device, StagingBufferMemory, 0, ImageSize, 0, &Data);
    memcpy(Data, Pixels, static_cast<size_t>(ImageSize));
    vkUnmapMemory(Vulkan::Renderer::Device, StagingBufferMemory);

    stbi_image_free(Pixels);

    Vulkan::Renderer::CreateImage(TextureWidth, TextureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Vulkan::Renderer::TextureImage, Vulkan::Renderer::TextureImageMemory);

    Vulkan::Renderer::TransitionImageLayout(Vulkan::Renderer::TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    Vulkan::Renderer::CopyBufferToImage(StagingBuffer, Vulkan::Renderer::TextureImage, static_cast<uint32_t>(TextureWidth), static_cast<uint32_t>(TextureHeight));
    Vulkan::Renderer::TransitionImageLayout(Vulkan::Renderer::TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(Vulkan::Renderer::Device, StagingBuffer, nullptr);
    vkFreeMemory(Vulkan::Renderer::Device, StagingBufferMemory, nullptr);
}

void Vulkan::Renderer::CreateTextureSampler()
{
    VkSamplerCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    CreateInfo.magFilter = VK_FILTER_LINEAR;
    CreateInfo.minFilter = VK_FILTER_LINEAR;
    CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties Properties{};
    vkGetPhysicalDeviceProperties(Vulkan::Renderer::PhysicalDevice, &Properties);

    CreateInfo.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;
    CreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    CreateInfo.unnormalizedCoordinates = VK_FALSE;
    CreateInfo.compareEnable = VK_FALSE;
    CreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    CreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateInfo.mipLodBias = 0.0f;
    CreateInfo.minLod = 0.0f;
    CreateInfo.maxLod = 0.0f;

    if (vkCreateSampler(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Vulkan::Renderer::TextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create texture sampler!");
    }
    else if (vkCreateSampler(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Vulkan::Renderer::TextureSampler) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created texture sampler!" << std::endl;
    }
}

void Vulkan::Renderer::CreateTextureImageView()
{   
    Vulkan::Renderer::TextureImageView = Vulkan::Renderer::CreateImageView(Vulkan::Renderer::TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView Vulkan::Renderer::CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags)
{
    VkImageViewCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    CreateInfo.image = Image;
    CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    CreateInfo.format = Format;
    CreateInfo.subresourceRange.aspectMask = AspectFlags;
    CreateInfo.subresourceRange.baseMipLevel = 0;
    CreateInfo.subresourceRange.levelCount = 1;
    CreateInfo.subresourceRange.baseArrayLayer = 0;
    CreateInfo.subresourceRange.layerCount = 1;

    VkImageView ImageView;

    if (vkCreateImageView(Vulkan::Renderer::Device, &CreateInfo, nullptr, &ImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create image view!");
    }
    //else if (vkCreateImageView(Vulkan::Renderer::Device, &CreateInfo, nullptr, &ImageView) == VK_SUCCESS)
    //{
    //    std::cout << "VK > Successfully created image view!" << std::endl;
    //}

    return ImageView;
}

void Vulkan::Renderer::CreateUniformBuffers()
{
    VkDeviceSize BufferSize = sizeof(Vulkan::Renderer::UniformBufferObject);
    
    Vulkan::Renderer::UniformBuffers.resize(Vulkan::Renderer::MaxFramesInFlight);
    Vulkan::Renderer::UniformBuffersMemory.resize(Vulkan::Renderer::MaxFramesInFlight);
    Vulkan::Renderer::UniformBuffersMapped.resize(Vulkan::Renderer::MaxFramesInFlight);

    for (size_t i = 0; i < Vulkan::Renderer::MaxFramesInFlight; i++)
    {
        Vulkan::Renderer::CreateBuffer(BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Vulkan::Renderer::UniformBuffers[i], Vulkan::Renderer::UniformBuffersMemory[i]);

        vkMapMemory(Vulkan::Renderer::Device, Vulkan::Renderer::UniformBuffersMemory[i], 0, BufferSize, 0, &Vulkan::Renderer::UniformBuffersMapped[i]);
    }
}

void Vulkan::Renderer::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> PoolSizes{};
    PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSizes[0].descriptorCount = static_cast<uint32_t>(Vulkan::Renderer::MaxFramesInFlight);

    PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    PoolSizes[1].descriptorCount = static_cast<uint32_t>(Vulkan::Renderer::MaxFramesInFlight);

    VkDescriptorPoolCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    CreateInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
    CreateInfo.pPoolSizes = PoolSizes.data();
    CreateInfo.maxSets = static_cast<uint32_t>(Vulkan::Renderer::MaxFramesInFlight);

    if (vkCreateDescriptorPool(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Vulkan::Renderer::DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create descriptor pool!");
    }
    else if (vkCreateDescriptorPool(Vulkan::Renderer::Device, &CreateInfo, nullptr, &Vulkan::Renderer::DescriptorPool) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created descriptor pool! \n";
    }
}

void Vulkan::Renderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> Layouts(Vulkan::Renderer::MaxFramesInFlight, Vulkan::Renderer::DescriptorSetLayout);

    VkDescriptorSetAllocateInfo AllocateInfo{};
    AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocateInfo.descriptorPool = Vulkan::Renderer::DescriptorPool;
    AllocateInfo.descriptorSetCount = static_cast<uint32_t>(Vulkan::Renderer::MaxFramesInFlight);
    AllocateInfo.pSetLayouts = Layouts.data();

    Vulkan::Renderer::DescriptorSets.resize(Vulkan::Renderer::MaxFramesInFlight);

    if (vkAllocateDescriptorSets(Vulkan::Renderer::Device, &AllocateInfo, Vulkan::Renderer::DescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to allocate descriptor sets!");
    }
    else
    {
        std::cout << "VK > Successfully allocated descriptor sets! \n";
    }

    for (size_t i = 0; i < Vulkan::Renderer::MaxFramesInFlight; i++)
    {
        VkDescriptorBufferInfo BufferInfo{};
        BufferInfo.buffer = Vulkan::Renderer::UniformBuffers[i];
        BufferInfo.offset = 0;
        BufferInfo.range = sizeof(Vulkan::Renderer::UniformBufferObject);

        VkDescriptorImageInfo ImageInfo{};
        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.imageView = Vulkan::Renderer::TextureImageView;
        ImageInfo.sampler = Vulkan::Renderer::TextureSampler;

        std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};
        DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[0].dstSet = Vulkan::Renderer::DescriptorSets[i];
        DescriptorWrites[0].dstBinding = 0;
        DescriptorWrites[0].dstArrayElement = 0;
        DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        DescriptorWrites[0].descriptorCount = 1;
        DescriptorWrites[0].pBufferInfo = &BufferInfo;
        //DescriptorWrites[0].pTexelBufferView = nullptr;

        DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[1].dstSet = Vulkan::Renderer::DescriptorSets[i];
        DescriptorWrites[1].dstBinding = 1;
        DescriptorWrites[1].dstArrayElement = 0;
        DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        DescriptorWrites[1].descriptorCount = 1;
        DescriptorWrites[1].pImageInfo = &ImageInfo;

        vkUpdateDescriptorSets(Vulkan::Renderer::Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
    }
}

void Vulkan::Renderer::UpdateUniformBuffer(uint32_t CurrentImage)
{
    static auto StartTime = std::chrono::high_resolution_clock::now();

    auto CurrentTime = std::chrono::high_resolution_clock::now();
    float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();

    Vulkan::Renderer::UniformBufferObject UBO{};
    //UBO.Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(8.0f, 0.0f, 0.0f));
    //UBO.Model = glm::mat4(1.0f);
    UBO.Model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, -5.0f, 5.0f));
    //UBO.Transform = glm::mat4(1.0f, 1.0f, 1.0f, 1.0f);
    //UBO.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    UBO.View = Engine::Camera::GetViewMatrix();
    UBO.Proj = glm::perspective(glm::radians(45.0f), Vulkan::Renderer::SwapChainExtent.width / (float)Vulkan::Renderer::SwapChainExtent.height, 0.1f, 256.0f);
    UBO.Proj[1][1] *= -1;

    memcpy(Vulkan::Renderer::UniformBuffersMapped[CurrentImage], &UBO, sizeof(UBO));
}

void Vulkan::Renderer::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
{
    VkBufferCreateInfo BufferInfo{};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = Usage;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Vulkan::Renderer::Device, &BufferInfo, nullptr, &Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to create buffer!");
    }
    else if (vkCreateBuffer(Vulkan::Renderer::Device, &BufferInfo, nullptr, &Buffer) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully created buffer! \n";
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetBufferMemoryRequirements(Vulkan::Renderer::Device, Buffer, &MemoryRequirements);

    VkMemoryAllocateInfo AllocateInfo{};
    AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocateInfo.allocationSize = MemoryRequirements.size;
    AllocateInfo.memoryTypeIndex = Vulkan::Renderer::FindMemoryType(MemoryRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(Vulkan::Renderer::Device, &AllocateInfo, nullptr, &BufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("VK > Failed to allocate buffer memory!");
    }
    else if (vkAllocateMemory(Vulkan::Renderer::Device, &AllocateInfo, nullptr, &BufferMemory) == VK_SUCCESS)
    {
        std::cout << "VK > Successfully allocated buffer memory! \n";
    }

    vkBindBufferMemory(Vulkan::Renderer::Device, Buffer, BufferMemory, 0);
}

VkCommandBuffer Vulkan::Renderer::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo AllocateInfo{};
    AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocateInfo.commandPool = Vulkan::Renderer::CommandPool;
    AllocateInfo.commandBufferCount = 1;

    VkCommandBuffer CommandBuffer;
    vkAllocateCommandBuffers(Vulkan::Renderer::Device, &AllocateInfo, &CommandBuffer);

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    return CommandBuffer;
}

void Vulkan::Renderer::EndSingleTimeCommands(VkCommandBuffer CommandBuffer)
{
    vkEndCommandBuffer(CommandBuffer);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    vkQueueSubmit(Vulkan::Renderer::GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Vulkan::Renderer::GraphicsQueue);

    vkFreeCommandBuffers(Vulkan::Renderer::Device, Vulkan::Renderer::CommandPool, 1, &CommandBuffer);
}

void Vulkan::Renderer::CopyBuffer(VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size)
{
    VkCommandBuffer CommandBuffer = Vulkan::Renderer::BeginSingleTimeCommands();

    VkBufferCopy CopyRegion{};
    CopyRegion.srcOffset = 0;
    CopyRegion.dstOffset = 0;
    CopyRegion.size = Size;

    vkCmdCopyBuffer(CommandBuffer, SrcBuffer, DstBuffer, 1, &CopyRegion);

    Vulkan::Renderer::EndSingleTimeCommands(CommandBuffer);
}

void Vulkan::Renderer::CopyBufferToImage(VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height)
{
    VkCommandBuffer CommandBuffer = Vulkan::Renderer::BeginSingleTimeCommands();

    VkBufferImageCopy Region{};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;

    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;

    Region.imageOffset = { 0, 0, 0 };
    Region.imageExtent = { Width, Height, 1 };

    vkCmdCopyBufferToImage(CommandBuffer, Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    Vulkan::Renderer::EndSingleTimeCommands(CommandBuffer);
}

uint32_t Vulkan::Renderer::FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
{
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(Vulkan::Renderer::PhysicalDevice, &MemoryProperties);

    for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++)
    {
        if (TypeFilter & (1 << i) && (MemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
        {
            std::cout << "VK > Found suitable memory type! \n";
            return i;
        }
    }

    throw std::runtime_error("VK > Failed to find suitable memory type!");
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

/*
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
*/