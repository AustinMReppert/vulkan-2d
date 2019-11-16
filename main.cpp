#include "Renderer.h"

#include <GLFW/glfw3.h>

#include <iostream>

int main() {

  Renderer renderer;
  renderer.createWindow();

  renderer.enableRequiredExtensions();
  renderer.enableRequiredLayers();
  renderer.initVk();
  renderer.createSurface();
  renderer.pickDevice();
  renderer.createSwapChain();
  renderer.createShaders();
  renderer.createRenderPass();
  renderer.createPipeline();
  while (!glfwWindowShouldClose(renderer.window->glfwWindow)) {
    glfwPollEvents();
    break;
  }
  std::cout << "exiting" << std::endl;
  renderer.cleanup();
  return 0;
}