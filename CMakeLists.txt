cmake_minimum_required(VERSION 3.20)

project(UVMappingEntrega4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(UVMappingEntrega4
    src/main.cpp
    src/VulkanApp.cpp
)

target_link_libraries(UVMappingEntrega4
    PRIVATE
        Vulkan::Vulkan
        glfw
)