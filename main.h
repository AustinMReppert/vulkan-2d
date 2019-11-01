#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#ifndef VULKAN_MAIN_H
#define VULKAN_MAIN_H

#include <iostream>
#include <memory>

#include <vulkan/vulkan.hpp>

// This must be included after vulkan.hpp
#include "Window.h"

#include "SwapChainUtils.h"

#include <GLFW/glfw3.h>
#include <set>
#include <vector>
#include <experimental/vector>
#include <string_view>

vk::UniqueInstance vkInstance;
vk::PhysicalDevice physicalDevice;
vk::UniqueDevice logicalDevice;
std::vector<const char *> enabledExtensions;
std::vector<const char *> enabledDeviceExtensions;
std::vector<const char *> enabledLayers;
std::vector<vk::Image> swapChainImages;
std::vector<vk::UniqueImageView> swapChainImageViews;
vk::Queue graphicsQueue;
vk::Queue presentQueue;
vk::SurfaceKHR surface;
vk::UniqueSurfaceKHR uniqueSurface;
vk::UniqueSwapchainKHR swapChain;
vk::Extent2D optimalExtent;
vk::PresentModeKHR optimalPresentMode;
vk::SurfaceFormatKHR optimalSurfaceFormat;

SwapChainSupportDetails swapChainSupportDetails;

uint32_t graphicsQueueFamilyIndex;
uint32_t presentQueueFamilyIndex;

std::unique_ptr<Window> window;

void initVk();

void enableRequiredExtensions();

void enableRequiredDeviceExtensions(vk::PhysicalDevice device);

void enableRequiredLayers();

void pickDevice();

void createWindow();

void createSurface();

void createSwapChain();

void cleanup();

int main();

#endif

#pragma clang diagnostic pop