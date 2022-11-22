cmake_minimum_required(VERSION 3.16)

project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 20)

find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(VulkanRenderer "Main.cpp")
target_link_libraries(VulkanRenderer PRIVATE glm
                                             Vulkan::Vulkan
                                             glfw)