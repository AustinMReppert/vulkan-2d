#ifndef VULKAN_DEVICEUTILS_H
#define VULKAN_DEVICEUTILS_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>
#include <experimental/vector>
#include <optional>

class DeviceUtils {

public:

  static vk::PhysicalDevice getOptimalPhysicalDevice() {
    vk::PhysicalDevice physicalDevice;
    auto physicalDevices = vkInstance->enumeratePhysicalDevices();
    std::experimental::erase_if(physicalDevices,
                                [](const auto& physicalDevice) {
                                  return physicalDevice.getProperties().deviceType !=
                                         vk::PhysicalDeviceType::eDiscreteGpu;
                                });

    physicalDevice = physicalDevices.front();
    return physicalDevice;
  }

  /**
   * Returns a set containing graphics and present queue family indices.
   * @param physicalDevice The physical device to to select queue families from.
   * @param surface The surface which will be presenting to.
   * @return A set containing graphics and present queue family indices.
   */
  static std::set<std::optional<uint32_t>>
  getGraphicsAndPresentQueueFamilyIndices(const vk::PhysicalDevice& physicalDevice,
                                          const vk::UniqueSurfaceKHR& surface) {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    uint32_t graphicsQueueFamilyIndex = queueFamilyProperties.size();
    uint32_t presentQueueFamilyIndex = queueFamilyProperties.size();
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i) {
      if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
        graphicsQueueFamilyIndex = i;
      if (physicalDevice.getSurfaceSupportKHR(i, surface.get()))
        presentQueueFamilyIndex = i;
      if (graphicsQueueFamilyIndex == presentQueueFamilyIndex) break;
      ++i;
    }
    std::set<std::optional<uint32_t>> ret;
    if (graphicsQueueFamilyIndex != static_cast<uint32_t>(queueFamilyProperties.size()))
      ret.insert({graphicsQueueFamilyIndex});
    else ret.insert({});
    if (presentQueueFamilyIndex != static_cast<uint32_t>(queueFamilyProperties.size()))
      ret.insert({presentQueueFamilyIndex});
    else ret.insert({});
    return ret;
  }

private:
protected:

};

#endif