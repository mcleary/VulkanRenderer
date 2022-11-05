
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

class SimpleVulkanApplication
{
public:
    void Run()
    {
        InitWindow();
        InitVulkan();
        MainLoop();
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

    void InitVulkan()
    {
        const std::vector<vk::ExtensionProperties> VulkanInstanceExtentionProperties = vk::enumerateInstanceExtensionProperties();
        for (const vk::ExtensionProperties& Property : VulkanInstanceExtentionProperties)
        {
            std::cout << Property.extensionName << std::endl;
        }
        std::cout << std::endl;

        constexpr vk::ApplicationInfo AppInfo
        {
            AppName.data(),
            VK_MAKE_VERSION(0, 0, 0),
            nullptr,
            0,
            VK_API_VERSION_1_1
        };

        const std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> Extensions = {};
        vk::InstanceCreateInfo InstanceCreateInfo
        {
            vk::InstanceCreateFlags{},
            &AppInfo,
            Layers,
            Extensions,
        };

        VulkanInstance = vk::createInstance(InstanceCreateInfo);

        const std::vector<vk::PhysicalDevice> PhysicalDevices = VulkanInstance.enumeratePhysicalDevices();
        for (const vk::PhysicalDevice& Device : PhysicalDevices)
        {
            vk::PhysicalDeviceProperties DeviceProperties = Device.getProperties();

            std::cout << "Device Name: " << DeviceProperties.deviceName << std::endl;
            const uint32_t ApiVersion = DeviceProperties.apiVersion;
            std::cout << "Vulkan Version : " << VK_VERSION_MAJOR(ApiVersion) << "." << VK_VERSION_MINOR(ApiVersion) << "." << VK_VERSION_PATCH(ApiVersion) << std::endl;
            vk::PhysicalDeviceLimits DeviceLimits = DeviceProperties.limits;
            std::cout << "Max Compute Shared Memory Size: " << DeviceLimits.maxComputeSharedMemorySize / 1024 << " KB" << std::endl;
        }
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
        VulkanInstance.destroy();

        glfwDestroyWindow(Window);

        glfwTerminate();
    }

private:
    constexpr static glm::ivec2 WindowSize{ 800, 600 };
    constexpr static std::string_view AppName = "\"Simple\" Vulkan Renderer";

    GLFWwindow* Window;

    vk::Instance VulkanInstance;
};

int main(int ArgC, char* ArgV[])
{
    SimpleVulkanApplication VulkanApp;

    try
    {
        VulkanApp.Run();
    }
    catch (const std::exception& Exception)
    {
        std::cerr << Exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}