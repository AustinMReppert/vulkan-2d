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

  static std::vector<vk::UniqueImageView>
  getImageViews(const vk::UniqueDevice& device, const std::vector<vk::Image>& images, vk::Format format) {
    std::vector<vk::UniqueImageView> imageViews(images.size());
    for (const auto& image : images) {
      vk::ImageViewCreateInfo createInfo = {{}, image, vk::ImageViewType::e2D, format, {},
                                            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
      imageViews.push_back(device->createImageViewUnique(createInfo));
    }
    return imageViews;
  }

  static vk::UniqueSwapchainKHR
  createSwapChain(const vk::UniqueDevice& device,
                  const SwapChainSupportDetails& swapChainSupportDetails, const vk::UniqueSurfaceKHR& surface,
                  vk::PresentModeKHR presentMode,
                  vk::SurfaceFormatKHR surfaceFormat, vk::Extent2D extent, uint32_t graphicsQueueFamily,
                  uint32_t presentQueueFamily) {
    bool sharedQueues = graphicsQueueFamily == presentQueueFamily;
    std::array<uint32_t, 2> queueFamilyIndices = {graphicsQueueFamily, presentQueueFamily};
    vk::SwapchainCreateInfoKHR createInfo = {{}, surface.get(),
                                             std::clamp(swapChainSupportDetails.capabilities.minImageCount + 1,
                                                        swapChainSupportDetails.capabilities.minImageCount,
                                                        swapChainSupportDetails.capabilities.maxImageCount),
                                             surfaceFormat.format,
                                             surfaceFormat.colorSpace,
                                             extent,
                                             1,
                                             vk::ImageUsageFlagBits::eColorAttachment,
                                             sharedQueues ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
                                             static_cast<uint32_t>(sharedQueues ? 0 : 2),
                                             sharedQueues ? nullptr : queueFamilyIndices.data(),
                                             swapChainSupportDetails.capabilities.currentTransform,
                                             vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                             presentMode, VK_TRUE, nullptr
    };
    vk::UniqueSwapchainKHR swapChain;
    try {
      swapChain = device->createSwapchainKHRUnique(createInfo);
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
      exit(-1);
    }
    return swapChain;
  }

  static SwapChainSupportDetails getSwapChainSupport(vk::PhysicalDevice device, const vk::UniqueSurfaceKHR& surface) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface.get());
    details.formats = device.getSurfaceFormatsKHR(surface.get());
    details.presentModes = device.getSurfacePresentModesKHR(surface.get());
    return details;
  }

  static vk::SurfaceFormatKHR getOptimalSurfaceFormat(const SwapChainSupportDetails& swapChainSupportDetails) {
    for (const auto& format : swapChainSupportDetails.formats)
      if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && format.format == vk::Format::eB8G8R8A8Unorm)
        return format;
    return swapChainSupportDetails.formats.front();
  }

  static vk::Extent2D
  getOptimalExtent(const SwapChainSupportDetails& swapChainSupportDetails, const VkExtent2D& preferredExtent) {
    // the window manager lets you choose
    if (swapChainSupportDetails.capabilities.currentExtent == std::numeric_limits<uint32_t>::max())
      return vk::Extent2D(std::clamp(preferredExtent.width,
                                     swapChainSupportDetails.capabilities.minImageExtent.width,
                                     swapChainSupportDetails.capabilities.maxImageExtent.width),
                          std::clamp(preferredExtent.height,
                                     swapChainSupportDetails.capabilities.minImageExtent.height,
                                     swapChainSupportDetails.capabilities.maxImageExtent.height));
    else return swapChainSupportDetails.capabilities.currentExtent;
  }

  static vk::PresentModeKHR getOptimalPresentMode(const SwapChainSupportDetails& swapChainSupportDetails) {
    auto curr = *std::max_element(PRESENT_MODE_PRIORITIES.begin(), PRESENT_MODE_PRIORITIES.end(),
                                  [](const auto& p1, const auto& p2) -> int { return p1.second < p2.second; });
    for (const auto& presentMode : swapChainSupportDetails.presentModes)
      for (const auto& presentPriority : PRESENT_MODE_PRIORITIES)
        if (presentMode == presentPriority.first && presentPriority.second < curr.second)
          curr = presentPriority;
    return curr.first;
  }

private:
  static constexpr std::array<std::pair<vk::PresentModeKHR, uint32_t>, 4> PRESENT_MODE_PRIORITIES = {
      std::pair<vk::PresentModeKHR, uint32_t>{vk::PresentModeKHR::eMailbox, 0},
      {vk::PresentModeKHR::eFifoRelaxed, 1},
      {vk::PresentModeKHR::eFifo, 2},
      {vk::PresentModeKHR::eImmediate, std::numeric_limits<uint32_t>::max()}};

};


#endif
