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
  renderer.createFramebuffers();

  while(!renderer.window->isClosing()) {
    glfwPollEvents();
    break;
  }

#ifdef DEBUG
  std::cout << "exiting" << std::endl;
#endif
  renderer.cleanup();
  return 0;
}