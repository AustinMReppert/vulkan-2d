#include "Window.h"

#include <GLFW/glfw3.h>

#include <iostream>

Window::Window() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_FLOATING, true);
  glfwWindowHint(GLFW_MAXIMIZED, true);
  glfwWindow = glfwCreateWindow(200, 200, "Vulkan", nullptr, nullptr);
}
Window::Window(int width, int height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, true);
  glfwWindowHint(GLFW_FLOATING, false);
  glfwWindowHint(GLFW_MAXIMIZED, true);
  glfwWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
}


Window::~Window() {
  glfwDestroyWindow(glfwWindow);
}

int Window::getWidth() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);
  return width;
}

int Window::getHeight() {
  int width, height;
  glfwGetWindowSize(glfwWindow, &width, &height);
  return height;
}
