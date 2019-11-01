#include "main.h"

#ifndef NDEBUG
#define DEBUG
#endif

template<class T>
constexpr uint32_t vkSize(T t) {
  return static_cast<uint32_t>(t.size());
}

constexpr uint32_t vkMakeVersion(uint32_t major, uint32_t minor, uint32_t patch) {
  return major << static_cast<uint32_t>(22) | (minor << static_cast<uint32_t >(12)) | (patch);
}

void initVk() {
  try {
    vk::ApplicationInfo applicationInfo("vulkan", vkMakeVersion(1, 0, 0), "", vkMakeVersion(1, 0, 0),
                                        vkMakeVersion(1, 0, 0));
    vk::InstanceCreateInfo createInfo({}, &applicationInfo, vkSize(enabledLayers), enabledLayers.data(),
                                      vkSize(enabledExtensions), enabledExtensions.data());
    vkInstance = vk::createInstanceUnique(createInfo);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    exit(-1);
  }

#ifdef DEBUG
  std::cout << "vulkan initialized" << std::endl;
#endif

}

void enableRequiredDeviceExtensions(vk::PhysicalDevice device) {
  enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::vector<vk::ExtensionProperties> deviceExtensions = device.enumerateDeviceExtensionProperties();
  std::vector<const char *> deviceExtensionNames(deviceExtensions.size());
  std::transform(deviceExtensions.begin(), deviceExtensions.end(), deviceExtensionNames.begin(),
                 [](const auto& i) -> const char * { return i.extensionName; });

#ifdef DEBUG
  std::cout << "Supported device extensions:" << std::endl;
  for (const auto& ext : deviceExtensions)
    std::cout << "\t" << ext.extensionName << std::endl;
#endif

  auto unsupportedExtensions = enabledDeviceExtensions;
  std::experimental::erase_if(unsupportedExtensions, [&](const auto& i) {
    return
        std::find_if(deviceExtensionNames.begin(),
                     deviceExtensionNames.end(),
                     [=](const auto& deviceExtensionName) {
                       return !std::strcmp(i, deviceExtensionName);
                     }) != deviceExtensionNames.end();
  });

  if (!unsupportedExtensions.empty())
    std::cout << "Missing Device Extensions:" << std::endl;
  for (const auto& i : unsupportedExtensions)
    std::cout << "\t" << i << std::endl;
  if (!unsupportedExtensions.empty())
    exit(-1);
}

void enableRequiredExtensions() {
  std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

#ifdef DEBUG
  std::cout << "supported extensions:" << std::endl;
  for (const auto& ext : supportedExtensions)
    std::cout << "\t" << ext.extensionName << std::endl;
#endif

  std::size_t numExtensions = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(reinterpret_cast<uint32_t *>(&numExtensions));
  for (std::size_t i = 0; i < numExtensions; ++i)
    enabledExtensions.push_back(glfwExtensions[i]);

#ifdef DEBUG
  std::cout << "GLFW requested the following extensions: " << std::endl;
  for (const auto& ext : enabledExtensions)
    std::cout << "\t" << ext << std::endl;
#endif

  for (const auto& reqExt : enabledExtensions) {
    bool found = false;
    for (const auto& ext : supportedExtensions) {
      if (!std::strcmp(ext.extensionName, reqExt)) {
        found = true;
        break;
      }
    }
    if (!found) {
      std::cerr << reqExt << " not found, quiting." << std::endl;
      cleanup();
      exit(-1);
    }
  }
}

void enableRequiredLayers() {
  std::vector<vk::LayerProperties> vkLayerProps = vk::enumerateInstanceLayerProperties();

#ifdef DEBUG
  enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
  std::cout << "Supported Layers:" << std::endl;
  for (const auto& layerProps : vkLayerProps)
    std::cout << "\t" << layerProps.layerName << std::endl;
#endif

}

void pickDevice() {
  auto physicalDevices = vkInstance->enumeratePhysicalDevices();
  std::experimental::erase_if(physicalDevices,
                              [](const auto& physicalDevice) {
                                return physicalDevice.getProperties().deviceType !=
                                       vk::PhysicalDeviceType::eDiscreteGpu;
                              });

  physicalDevice = physicalDevices.front();

#ifdef DEBUG
  std::cout << "Physical Devices: " << std::endl;
  for (auto& device : physicalDevices)
    std::cout << "\t" << device.getProperties().deviceName << std::endl;
#endif

  std::vector<vk::QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();
  uint32_t graphicsFamilyIndex = vkSize(queueFamilyProps), presentFamilyIndex = vkSize(queueFamilyProps);
  for (std::size_t i = 0; i < queueFamilyProps.size(); ++i) {
    bool supportsGraphics = false;
    bool supportsPresenting = false;
    if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
      supportsGraphics = true;
      graphicsFamilyIndex = static_cast<uint32_t>(i);
    }
    if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), uniqueSurface.get())) {
      supportsPresenting = true;
      presentFamilyIndex = static_cast<uint32_t>(i);
    }
    if (supportsGraphics && supportsPresenting) break;
  }

#ifdef DEBUG
  std::cout << "Queue Families:" << std::endl;
  for (std::size_t i = 0; i < queueFamilyProps.size(); ++i) {
    std::cout << "\tindex: " << i << std::endl;
    std::cout << "\tgraphics: "
              << static_cast<bool>(queueFamilyProps[static_cast<uint32_t>(i)].queueFlags & vk::QueueFlagBits::eGraphics)
              << std::endl;
    std::cout << "\tpresentation: "
              << physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), uniqueSurface.get()) << std::endl;
  }
#endif

  if (graphicsFamilyIndex >= queueFamilyProps.size() || presentFamilyIndex >= queueFamilyProps.size()) {
    std::cerr << "Could not find a queue family supporting graphics and/or presenting." << std::endl;
    cleanup();
    exit(-1);
  }

  graphicsQueueFamilyIndex = graphicsFamilyIndex;
  presentQueueFamilyIndex = presentFamilyIndex;

  enableRequiredDeviceExtensions(physicalDevice);
  std::set<uint32_t> uniqueQueueFamilies = {graphicsFamilyIndex, presentFamilyIndex};
  std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
  std::vector<float *> queuePriorities;
  for (const auto& queueFamily : uniqueQueueFamilies) {
    float priority = 1;
    deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, queueFamily, 1, &priority));
  }

  logicalDevice = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo({}, vkSize(deviceQueueCreateInfos),
                                                                         deviceQueueCreateInfos.data(),
                                                                         vkSize(enabledLayers), enabledLayers.data(),
                                                                         vkSize(enabledDeviceExtensions),
                                                                         enabledDeviceExtensions.data()));

  graphicsQueue = logicalDevice->getQueue(graphicsFamilyIndex, 0);
  presentQueue = logicalDevice->getQueue(presentFamilyIndex, 0);

#ifdef DEBUG
  std::cout << "Selecting queue " << graphicsFamilyIndex << " for graphics" << std::endl;
  std::cout << "Selecting queue " << presentFamilyIndex << " for presenting" << std::endl;
#endif

}

void createSwapChain() {
  vk::Extent2D preferredExtent(window->getWidth(), window->getHeight());
  swapChainSupportDetails = SwapChainUtils::getSwapChainSupport(physicalDevice, uniqueSurface);
  optimalExtent = SwapChainUtils::getOptimalExtent(swapChainSupportDetails, preferredExtent);
  optimalPresentMode = SwapChainUtils::getOptimalPresentMode(swapChainSupportDetails);
  optimalSurfaceFormat = SwapChainUtils::getOptimalSurfaceFormat(swapChainSupportDetails);

  swapChain = SwapChainUtils::createSwapChain(logicalDevice, swapChainSupportDetails, uniqueSurface,
                                              optimalPresentMode,
                                              optimalSurfaceFormat, optimalExtent, graphicsQueueFamilyIndex,
                                              presentQueueFamilyIndex);
  for (const auto& swapChainImage : logicalDevice->getSwapchainImagesKHR(swapChain.get()))
    swapChainImages.push_back(swapChainImage);
  swapChainImageViews = SwapChainUtils::getImageViews(logicalDevice, swapChainImages, optimalSurfaceFormat.format);

#ifdef DEBUG
  std::cout << "Supported Present Modes:" << std::endl;
  for (const auto& presentMode : swapChainSupportDetails.presentModes)
    std::cout << "\t" << vk::to_string(presentMode) << std::endl;
  std::cout << "Supported Surface Formats:" << std::endl;
  for (const auto& format : swapChainSupportDetails.formats) {
    std::cout << "\tformat: " << vk::to_string(format.format) << std::endl;
    std::cout << "\tcolor space: " << vk::to_string(format.colorSpace) << std::endl;
  }

  std::cout << "Selected Surface Extent: " << optimalExtent.width << " X " << optimalExtent.height << std::endl;
  std::cout << "Selected Surface Present Mode: " << vk::to_string(optimalPresentMode) << std::endl;
  std::cout << "Selected Surface Color Space: " << vk::to_string(optimalSurfaceFormat.colorSpace) << std::endl;
  std::cout << "Selected Surface Format: " << vk::to_string(optimalSurfaceFormat.format) << std::endl;

  std::cout << "Swap Chain created " << std::endl;
  std::cout << "Received " << swapChainImages.size() << " images for the swap chain" << std::endl;
#endif

}

void createWindow() {
  glfwInit();
  window = std::make_unique<Window>(800, 600);
}

void createSurface() {
  auto res = static_cast<vk::Result>(glfwCreateWindowSurface(static_cast<VkInstance>(vkInstance.get()),
                                                             window->glfwWindow, nullptr,
                                                             reinterpret_cast<VkSurfaceKHR *>(&surface)));
  if (res != vk::Result::eSuccess) {
    std::cerr << "Could not create surface to render to" << std::endl;
    cleanup();
    exit(-1);
  } else {
    uniqueSurface = vk::UniqueSurfaceKHR(surface, vkInstance.get());
  }
}

void cleanup() {
  glfwTerminate();
}

int main() {
  createWindow();

  enableRequiredExtensions();
  enableRequiredLayers();
  initVk();
  createSurface();
  pickDevice();
  createSwapChain();
  while (!glfwWindowShouldClose(window->glfwWindow)) {
    glfwPollEvents();
  }

  cleanup();
  return 0;
}
