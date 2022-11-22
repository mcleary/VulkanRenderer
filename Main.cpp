
#include <iostream>
#include <optional>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance Instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDebugUtilsMessengerEXT* pMessenger)
{
    return pfnVkCreateDebugUtilsMessengerEXT(Instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT Messenger, VkAllocationCallbacks const* pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(Instance, Messenger, pAllocator);
}

class SimpleVulkanApplication
{
public:
    void Run()
    {
        InitWindow();
        InitVulkan();
        // MainLoop();
        Shutdown();
    }

private:
    void InitWindow()
    {
        if (glfwInit() != GLFW_TRUE)
        {
            throw std::exception("Error initializing GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        Window = glfwCreateWindow(WindowSize.x, WindowSize.y, AppName.data(), nullptr, nullptr);

        if (Window == nullptr)
        {
            throw std::exception("Failed to destroy window");
        }
    }

    std::vector<const char*> GetRequiredVulkanExtensions() const
    {
        uint32_t GLFWExtensionCount = 0;
        const char** GLFWExtensions = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

        std::vector<const char*> Extensions{ GLFWExtensions, GLFWExtensions + GLFWExtensionCount };

        Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return Extensions;
    }

    void CreateVulkanInstance()
    {
        constexpr vk::ApplicationInfo AppInfo
        {
            AppName.data(),
            VK_MAKE_VERSION(0, 0, 0),
            nullptr,
            0,
            VK_API_VERSION_1_1
        };

        const std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> Extensions = GetRequiredVulkanExtensions();
        const vk::InstanceCreateInfo InstanceCreateInfo
        {
            vk::InstanceCreateFlags{},
            &AppInfo,
            Layers,
            Extensions,
        };

        constexpr vk::DebugUtilsMessageSeverityFlagsEXT SeverityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
        constexpr vk::DebugUtilsMessageTypeFlagsEXT MessageTypeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        const vk::DebugUtilsMessengerCreateInfoEXT DebugUtilsCreateInfo
        {
            vk::DebugUtilsMessengerCreateFlagsEXT{},
            SeverityFlags,
            MessageTypeFlags,
            &VulkanDebugCallback,
            reinterpret_cast<void*>(this)
        };

        const vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> InstanceCreateInfoChain(InstanceCreateInfo, DebugUtilsCreateInfo);

        VulkanInstance = vk::createInstance(InstanceCreateInfoChain.get<vk::InstanceCreateInfo>());

        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(VulkanInstance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        if (pfnVkCreateDebugUtilsMessengerEXT == nullptr)
        {
            throw std::exception("Unable to load PFN_vkCreateDebugUtilsMessengerEXT");
        }

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(VulkanInstance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
        if (pfnVkDestroyDebugUtilsMessengerEXT == nullptr)
        {
            throw std::exception("Unable to load PFN_vkDestroyDebugUtilsMessengerEXT");
        }

        VulkanDebugUtilsMessenger = VulkanInstance.createDebugUtilsMessengerEXT(DebugUtilsCreateInfo);
    }

    void CreateVulkanSurface()
    {
        const vk::Win32SurfaceCreateInfoKHR SurfaceCreateInfo
        {
            vk::Win32SurfaceCreateFlagBitsKHR{},
            GetModuleHandle(nullptr),
            glfwGetWin32Window(Window),
        };

        Surface = VulkanInstance.createWin32SurfaceKHR(SurfaceCreateInfo);
    }

    void PickVulkanPhysicalDevice()
    {
        const std::vector<vk::PhysicalDevice> PhysicalDevices = VulkanInstance.enumeratePhysicalDevices();
        for (const vk::PhysicalDevice& Device : PhysicalDevices)
        {
            vk::PhysicalDeviceProperties DeviceProperties = Device.getProperties();
            vk::PhysicalDeviceFeatures DeviceFeatures = Device.getFeatures();

            if (DeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
                DeviceFeatures.geometryShader)
            {
                std::cout << "Device Name: " << DeviceProperties.deviceName << std::endl;
                const uint32_t ApiVersion = DeviceProperties.apiVersion;
                std::cout << "Vulkan Version : " << VK_VERSION_MAJOR(ApiVersion) << "." << VK_VERSION_MINOR(ApiVersion) << "." << VK_VERSION_PATCH(ApiVersion) << std::endl;
                vk::PhysicalDeviceLimits DeviceLimits = DeviceProperties.limits;
                std::cout << "Max Compute Shared Memory Size: " << DeviceLimits.maxComputeSharedMemorySize / 1024 << " KB" << std::endl;
                std::cout << "Max Push Constants Memory Size : " << DeviceLimits.maxPushConstantsSize << " B" << std::endl;

                VulkanPhysicalDevice = Device;
                break;
            }
        }

        if (!VulkanPhysicalDevice)
        {
            throw std::runtime_error("Failed to find vulkan physical device");
        }
    }

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsQueueFamily;
        std::optional<uint32_t> PresentationFamilhy;
        std::optional<uint32_t> ComputeQueueFamily;
        std::optional<uint32_t> TransferQueueFamily;

        bool IsComplete() const
        {
            return GraphicsQueueFamily.has_value()
                && PresentationFamilhy.has_value()
                && ComputeQueueFamily.has_value()
                && TransferQueueFamily.has_value();
        }
    };

    QueueFamilyIndices FindQueueFamilyIndices() const
    {
        QueueFamilyIndices Indices;

        const std::vector<vk::QueueFamilyProperties> QueueFamilyProps = VulkanPhysicalDevice.getQueueFamilyProperties();
        for (size_t QueueIndex = 0; QueueIndex < QueueFamilyProps.size(); ++QueueIndex)
        {
            const vk::QueueFamilyProperties& Props = QueueFamilyProps[QueueIndex];
            if (Props.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                Indices.GraphicsQueueFamily = static_cast<uint32_t>(QueueIndex);
            }

            if (VulkanPhysicalDevice.getSurfaceSupportKHR(Indices.GraphicsQueueFamily.value(), Surface))
            {
                Indices.PresentationFamilhy = static_cast<uint32_t>(QueueIndex);
            }

            if (Props.queueFlags & vk::QueueFlagBits::eCompute)
            {
                Indices.ComputeQueueFamily = static_cast<uint32_t>(QueueIndex);
            }

            if (Props.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                Indices.TransferQueueFamily = static_cast<uint32_t>(QueueIndex);
            }

            if (Indices.IsComplete())
            {
                break;
            }
        }

        return Indices;
    }

    void CreateVulkanDevice()
    {
        const QueueFamilyIndices Indices = FindQueueFamilyIndices();

        if (!Indices.GraphicsQueueFamily.has_value())
        {
            throw std::exception("Physical device doesn't support graphics queues");
        }

        const float QueuePriority = 1.0f;
        const vk::DeviceQueueCreateInfo DeviceQueueCreateInfo
        {
            vk::DeviceQueueCreateFlags{},
            Indices.GraphicsQueueFamily.value(),
            1,
            &QueuePriority
        };

        const vk::DeviceCreateInfo DeviceCreateInfo
        {
            vk::DeviceCreateFlags{},
            DeviceQueueCreateInfo
        };

        VulkanDevice = VulkanPhysicalDevice.createDevice(DeviceCreateInfo);
        VulkanGraphicsQueue = VulkanDevice.getQueue(Indices.GraphicsQueueFamily.value(), 0);
        VulkanPresentationQueue = VulkanDevice.getQueue(Indices.PresentationFamilhy.value(), 0);
    }

    void InitVulkan()
    {
        CreateVulkanInstance();
        CreateVulkanSurface();
        PickVulkanPhysicalDevice();
        CreateVulkanDevice();
    }

    void MainLoop()
    {
        while (!glfwWindowShouldClose(Window))
        {
            glfwPollEvents();
        }
    }

    void Shutdown()
    {
        VulkanDevice.destroy();
        VulkanInstance.destroySurfaceKHR(Surface);
        VulkanInstance.destroyDebugUtilsMessengerEXT(VulkanDebugUtilsMessenger);
        VulkanInstance.destroy();

        glfwDestroyWindow(Window);

        glfwTerminate();
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT MessageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
                                                              void* UserData)
    {
        const std::string MessageSeverityStr = vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(MessageSeverity));
        const std::string MessageTypeStr = vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(MessageType));
        std::cerr << "[" << MessageSeverityStr << "] " << "[" << MessageTypeStr << "] \n";

        for (uint32_t ObjectIndex = 0; ObjectIndex < CallbackData->objectCount; ++ObjectIndex)
        {
            const VkDebugUtilsObjectNameInfoEXT& Object = CallbackData->pObjects[ObjectIndex];
            const std::string ObjectTypeStr = vk::to_string(static_cast<vk::ObjectType>(Object.objectType));

            std::cerr << "\t Object " << ObjectIndex << " [ 0x" << std::hex << Object.objectHandle << " ] [" << ObjectTypeStr << " ]";

            if (Object.pObjectName)
            {
                std::cerr << "[ " << Object.pObjectName << " ] ";
            }

            std::cerr << "\n";
        }

        for (uint32_t LabelIndex = 0; LabelIndex < CallbackData->cmdBufLabelCount; ++LabelIndex)
        {
            const char* LabelName = CallbackData->pCmdBufLabels[LabelIndex].pLabelName;
            std::cerr << "\t Cmd Buffer Label " << LabelIndex << " [ " << LabelName << " ] \n";
        }

        for (uint32_t LabelIndex = 0; LabelIndex < CallbackData->queueLabelCount; ++LabelIndex)
        {
            const char* LabelName = CallbackData->pQueueLabels[LabelIndex].pLabelName;
            std::cerr << "\t Queue Label " << LabelIndex << " [ " << LabelName << " ] \n";
        }

        std::cerr << "\n\t " << CallbackData->pMessage << "\n" << std::endl;

        return VK_FALSE;
    }

private:
    constexpr static glm::ivec2 WindowSize{ 800, 600 };
    constexpr static std::string_view AppName = "\"Simple\" Vulkan Renderer";

    GLFWwindow* Window;

    constexpr static bool bEnableVulkanValidationLayers = true;
    vk::Instance VulkanInstance;
    vk::DebugUtilsMessengerEXT VulkanDebugUtilsMessenger;
    vk::PhysicalDevice VulkanPhysicalDevice;
    vk::Device VulkanDevice;
    vk::Queue VulkanGraphicsQueue;
    vk::Queue VulkanPresentationQueue;
    vk::SurfaceKHR Surface;
};

int main(int ArgC, char* ArgV[])
{
    SimpleVulkanApplication VulkanApp;

    try
    {
        VulkanApp.Run();
    }
    catch (const vk::SystemError& Exception)
    {
        std::cerr << "vk::SystemError: " << Exception.what() << std::endl;
    }
    catch (const std::exception& Exception)
    {
        std::cerr << "std::exception:  " << Exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}