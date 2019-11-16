cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(vulkan)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_executable(vulkan Renderer.cpp Window.h Window.cpp SwapChainUtils.h DeviceUtils.h Renderer.h VkUtils.h ShaderUtils.h Macros.h main.cpp)

target_compile_features(vulkan PUBLIC cxx_std_20)

target_include_directories(vulkan PRIVATE glm)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
if (WIN32)
  add_subdirectory($ENV{VULKAN_SDK}/shaderc ${CMAKE_CURRENT_BINARY_DIR}/shaderc)
elseif (UNIX)
  add_subdirectory($ENV{VULKAN_SDK}/../source/shaderc/src ${CMAKE_CURRENT_BINARY_DIR}/shaderc)
else ()
  add_subdirectory(shaderc)
endif ()

find_package(Vulkan REQUIRED)

# Disable building of glfw documentation, tests, and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

target_link_libraries(vulkan PRIVATE Vulkan::Vulkan PRIVATE glfw PRIVATE shaderc)