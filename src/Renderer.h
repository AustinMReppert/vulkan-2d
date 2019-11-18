#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <experimental/vector>

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

// This must be included after vulkan.hpp
#include "Window.h"
#include "SwapChainUtils.h"
#include "VkUtils.h"

class Renderer {
public:
  std::unique_ptr<Window> window;

  vk::UniqueInstance vkInstance;
  vk::UniqueDevice logicalDevice;
  std::vector<vk::UniqueImageView> swapChainImageViews;
  vk::UniqueSurfaceKHR uniqueSurface;
  vk::UniqueSwapchainKHR swapChain;
  vk::UniqueShaderModule vertShaderModUnique;
  vk::UniqueShaderModule fragShaderModUnique;
  vk::UniquePipelineLayout pipelineLayoutUnique;
  vk::UniqueRenderPass renderPassUnique;
  vk::UniquePipeline graphicsPipeLineUnique;
  std::vector<vk::UniqueFramebuffer> frameBuffersUnique;
  vk::UniqueCommandPool commandPoolUnique;
  std::vector<vk::UniqueCommandBuffer> commandBuffersUnique;

  vk::PhysicalDevice physicalDevice;
  std::vector<const char *> enabledExtensions;
  std::vector<const char *> enabledDeviceExtensions;
  std::vector<const char *> enabledLayers;
  std::vector<vk::Image> swapChainImages;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::SurfaceKHR surface;
  vk::Extent2D optimalExtent;
  vk::PresentModeKHR optimalPresentMode;
  vk::SurfaceFormatKHR optimalSurfaceFormat;

  SwapChainSupportDetails swapChainSupportDetails;

  uint32_t graphicsQueueFamilyIndex;
  uint32_t presentQueueFamilyIndex;

  void initVk();

  void enableRequiredExtensions();

  void enableRequiredDeviceExtensions(vk::PhysicalDevice device);

  void enableRequiredLayers();

  void pickDevice();

  /**
   * Creates the window to render to.
   */
  void createWindow();

  /**
   * Creates a surface from the window.
   */
  void createSurface();

  void createSwapChain();

  /**
   * Creates the vertex and fragment shader modules from shaders/vertex.vert and shaders/fragment.frag.
   */
  void createShaders();

  /**
   * Creates the render pass used for the graphics pipeline.
   */
  void createRenderPass();

  /**
   * Creates the graphics pipeline.
   */
  void createPipeline();

  /**
   * Cleans up any resource that are not automatically freed.
   */
  void cleanup();

  void createFramebuffers();

  void createCommandPool();

  void createCommandBuffers();

  void render();

};

#endif