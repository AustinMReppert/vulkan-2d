# Make sure the user is useing an up to date cmake version
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# Specify the name of our project
project(vulkan)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Specify the name of our application and its source files
add_executable(vulkan main.cpp Window.h Window.cpp SwapChainUtils.h DeviceUtils.h main.h VkUtils.h ShaderUtils.h Macros.h)

# Enable c++ 20
target_compile_features(vulkan PUBLIC cxx_std_20)

# Add glm includes
target_include_directories(vulkan PRIVATE glm)

# Tell cmake to find the vulkan sdk
find_package(Vulkan REQUIRED)

if (UNIX)
  get_filename_component(vulkan_home $ENV{VULKAN_SDK} DIRECTORY)
  add_subdirectory(${vulkan_home}/source/shaderc/src ${CMAKE_CURRENT_BINARY_DIR}/shaderc)
elseif (WIN32)
  add_subdirectory($ENV{VULKAN_SDK}/shaderc)
else ()
  add_subdirectory(shaderc)
endif ()

# Add vulkan includes
target_include_directories(vulkan PRIVATE Vulkan::Vulkan PUBLIC ${shaderc_SOURCE_DIR}/libshaderc_util/include)

# Link against vulkan
target_link_libraries(vulkan Vulkan::Vulkan)

# Diable building of glfw documentation, tests, and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#set(SPIRV_HEADER_DIR ${SPIRV-Headers_SOURCE_DIR})

# Run cmake is glfw subdirectory
add_subdirectory(glfw)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)

# Link our application with glfw
target_link_libraries(vulkan glfw)
target_link_libraries(vulkan shaderc)
