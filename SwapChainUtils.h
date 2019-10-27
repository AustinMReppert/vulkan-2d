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

  static SwapChainSupportDetails getSwapChainSupport(vk::PhysicalDevice device, const vk::UniqueSurfaceKHR &surface) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface.get());
    details.formats = device.getSurfaceFormatsKHR(surface.get());
    details.presentModes = device.getSurfacePresentModesKHR(surface.get());
    return details;
  }

  static vk::SurfaceFormatKHR getOptimalSurfaceFormat(const SwapChainSupportDetails& swapChainSupportDetails) {
    for(const auto& format : swapChainSupportDetails.formats)
      if(format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && format.format == vk::Format::eB8G8R8A8Unorm)
        return format;
    return swapChainSupportDetails.formats.front();
  }

  static vk::Extent2D
  getOptimalExtent(const SwapChainSupportDetails &swapChainSupportDetails, const VkExtent2D &preferredExtent) {
    if (swapChainSupportDetails.capabilities.currentExtent == std::numeric_limits<uint32_t>::max()) {
      // the window manager lets you choose
      vk::Extent2D ret;
      // chooses the preferred width/height, if it is in bounds.
      ret.width = std::max(swapChainSupportDetails.capabilities.minImageExtent.width,
                           std::min(swapChainSupportDetails.capabilities.maxImageExtent.width, preferredExtent.width));
      ret.height = std::max(swapChainSupportDetails.capabilities.minImageExtent.height,
                            std::min(swapChainSupportDetails.capabilities.maxImageExtent.height,
                                     preferredExtent.height));
      return ret;
    } else return swapChainSupportDetails.capabilities.currentExtent;
  }

  static vk::PresentModeKHR getOptimalPresentMode(const SwapChainSupportDetails &swapChainSupportDetails) {
    auto curr = *std::max_element(PRESENT_MODE_PRIORITIES.begin(), PRESENT_MODE_PRIORITIES.end(),
                                  [] (const auto &p1, const auto &p2) -> int { return p1.second < p2.second; });
    for (const auto &presentMode : swapChainSupportDetails.presentModes)
      for(const auto& presentPriority : PRESENT_MODE_PRIORITIES)
        if(presentMode == presentPriority.first && presentPriority.second < curr.second)
          curr = presentPriority;
    return curr.first;
  }

private:
  static constexpr std::array<std::pair<vk::PresentModeKHR, uint32_t>, 4> PRESENT_MODE_PRIORITIES = {
      std::pair{vk::PresentModeKHR::eMailbox, 0},
      {vk::PresentModeKHR::eFifoRelaxed, 1},
      {vk::PresentModeKHR::eFifo, 2},
      {vk::PresentModeKHR::eImmediate, std::numeric_limits<uint32_t>::max()}};

};


#endif //VULKAN_SWAPCHAINUTILS_H
