#include "Window.h"

#include "Macros.h"

Window::Window() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, true);
  glfwWindowHint(GLFW_FLOATING, false);
  glfwWindowHint(GLFW_MAXIMIZED, true);
  glfwWindow = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

Window::Window(uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, true);
  glfwWindowHint(GLFW_FLOATING, false);
  glfwWindowHint(GLFW_MAXIMIZED, false);
  glfwWindow = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Vulkan", nullptr, nullptr);
}

void Window::zoom() {
  int maximized = glfwGetWindowAttrib(glfwWindow, GLFW_MAXIMIZED);
  if(!maximized)
    glfwMaximizeWindow(glfwWindow);
  else
    glfwRestoreWindow(glfwWindow);
}

void Window::minimize() {
  glfwIconifyWindow(glfwWindow);
}

void Window::setTitle(const std::string_view& title) {
  glfwSetWindowTitle(glfwWindow, title.data());
}

void Window::center() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);

  const GLFWvidmode *vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwSetWindowPos(glfwWindow, (vm->width - width) / 2, (vm->height - height) / 2);
}

Window::~Window() {
  glfwDestroyWindow(glfwWindow);
}

uint32_t Window::getWidth() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);
  return uint32(width);
}

uint32_t Window::getHeight() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);
  return uint32(height);
}
