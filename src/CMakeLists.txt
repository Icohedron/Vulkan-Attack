cmake_minimum_required(VERSION 3.10.2)

# Tell Vulkan which platform we're on
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DVK_USE_PLATFORM_WIN32_KHX -DWIN32_LEAN_AND_MEAN)

add_library(sample_layer SHARED vk_layer_dispatch_table.h sample_layer.cpp)
target_include_directories(sample_layer PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(sample_layer PRIVATE ".")

set_target_properties(sample_layer PROPERTIES CXX_STANDARD 17)

# Copy JSON manifest to the output folder
add_custom_command(TARGET sample_layer POST_BUILD 
               	COMMAND ${CMAKE_COMMAND} -E copy_if_different
               	"${CMAKE_SOURCE_DIR}/src/sample_layer_windows.json"
                $<TARGET_FILE_DIR:sample_layer>)

# Copy NSIS scripts to the output folder
add_custom_command(TARGET sample_layer POST_BUILD 
               	COMMAND ${CMAKE_COMMAND} -E copy_if_different
               	"${CMAKE_SOURCE_DIR}/src/sample_layer_silent_user_installer.nsi"
                $<TARGET_FILE_DIR:sample_layer>)
add_custom_command(TARGET sample_layer POST_BUILD 
               	COMMAND ${CMAKE_COMMAND} -E copy_if_different
               	"${CMAKE_SOURCE_DIR}/src/sample_layer_silent_vkconfig_override_replacer.nsi"
                $<TARGET_FILE_DIR:sample_layer>)