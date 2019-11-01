#include "Window.h"

#include <GLFW/glfw3.h>

Window::Window() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_FLOATING, true);
  glfwWindowHint(GLFW_MAXIMIZED, true);
  glfwWindow = glfwCreateWindow(200, 200, "Vulkan", nullptr, nullptr);
}

Window::Window(uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, true);
  glfwWindowHint(GLFW_FLOATING, false);
  glfwWindowHint(GLFW_MAXIMIZED, false);
  glfwWindow = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Vulkan", nullptr, nullptr);
}


Window::~Window() {
  glfwDestroyWindow(glfwWindow);
}

uint32_t Window::getWidth() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);
  return static_cast<uint32_t>(width);
}

uint32_t Window::getHeight() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);
  return static_cast<uint32_t>(height);
}
