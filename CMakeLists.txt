cmake_minimum_required(VERSION 3.10.2)

project(SampleLayer)

find_package(Vulkan)
message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")

add_subdirectory(src)
