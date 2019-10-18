#include "Window.h"

#include <GLFW/glfw3.h>

#include <iostream>

Window::Window() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_FLOATING, true);
  glfwWindow = glfwCreateWindow(200, 200, "Vulkan", nullptr, nullptr);
}
