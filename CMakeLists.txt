cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(vulkan)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_executable(vulkan src/Renderer.cpp src/Window.h src/Window.cpp src/SwapChainUtils.h src/DeviceUtils.h src/Renderer.h src/VkUtils.h src/ShaderUtils.h src/Macros.h src/main.cpp)

target_compile_features(vulkan PUBLIC cxx_std_20)

target_include_directories(vulkan PRIVATE third-party/glm)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
set(SKIP_GOOGLETEST_INSTALL ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

set( ON CACHE BOOL "" FORCE)
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
add_subdirectory(third-party/glfw)

target_link_libraries(vulkan PRIVATE Vulkan::Vulkan PRIVATE glfw PRIVATE shaderc)

file(COPY src/shaders DESTINATION ${CMAKE_BINARY_DIR})