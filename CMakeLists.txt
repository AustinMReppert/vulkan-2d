# Make sure the user is useing an up to date cmake version
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# Specify the name of our project
project(vulkan)

# Specify the name of our application and its source files
add_executable(vulkan main.cpp Window.h Window.cpp SwapChainUtils.h)

# Enable c++ 17
target_compile_features(vulkan PUBLIC cxx_std_17)

# Add glm includes
target_include_directories(vulkan PRIVATE glm)

# Tell cmake to find the vulkan sdk
find_package(Vulkan REQUIRED)

# Add vulkan includes
target_include_directories(vulkan PRIVATE Vulkan::Vulkan)

# Link against vulkan
target_link_libraries(vulkan Vulkan::Vulkan)

# Diable building of glfw documentation, tests, and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Run cmake is glfw subdirectory
add_subdirectory(glfw)

# Link our application with glfw
target_link_libraries(vulkan glfw)
