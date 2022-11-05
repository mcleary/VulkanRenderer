
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

constexpr glm::ivec2 WindowSize{ 800, 600 };
constexpr std::string_view AppName = "\"Simple\" Vulkan Renderer";

int main(int ArgC, char* ArgV[])
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Error initializing GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* Window = glfwCreateWindow(WindowSize.x, WindowSize.y, AppName.data(), nullptr, nullptr);

    if (Window == nullptr)
    {
        std::cerr << "Failed to destroy window" << std::endl;
        return -1;
    }

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

    vk::Instance VulkanInstance = vk::createInstance(InstanceCreateInfo);

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

    while (!glfwWindowShouldClose(Window))
    {
        glfwPollEvents();
    }

    VulkanInstance.destroy();

    glfwDestroyWindow(Window);

    glfwTerminate();

    return EXIT_SUCCESS;
}