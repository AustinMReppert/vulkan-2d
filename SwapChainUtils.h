#ifndef VULKAN_SWAPCHAINUTILS_H
#define VULKAN_SWAPCHAINUTILS_H

#include <vulkan/vulkan.hpp>

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChainUtils {

public:
  static SwapChainSupportDetails getSwapChainSupport(vk::PhysicalDevice device, const vk::UniqueSurfaceKHR& surface) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface.get());
    details.formats = device.getSurfaceFormatsKHR(surface.get());
    details.presentModes = device.getSurfacePresentModesKHR(surface.get());
    return details;
  }
};


#endif //VULKAN_SWAPCHAINUTILS_H
