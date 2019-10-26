#include <iostream>
#include <memory>

#include <vulkan/vulkan.hpp>

// This must be included after vulkan.hpp
#include "Window.h"

#include "SwapChainUtils.h"

#include <GLFW/glfw3.h>
#include <set>
#include <vector>

bool DEBUG = true;

vk::UniqueInstance vkInstance;
vk::PhysicalDevice physicalDevice;
vk::UniqueDevice logicalDevice;
std::vector<const char *> enabledExtensions;
std::vector<const char *> enabledDeviceExtensions;
std::vector<const char *> enabledLayers;
vk::Queue graphicsQueue;
vk::Queue presentQueue;
vk::SurfaceKHR surface;
vk::UniqueSurfaceKHR uniqueSurface;

SwapChainSupportDetails swapChainSupportDetails;

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

void initVk() {
  try {
    vk::ApplicationInfo applicationInfo("vulkan", VK_MAKE_VERSION(1, 0, 0), "", VK_MAKE_VERSION(1, 0, 0),
                                        VK_API_VERSION_1_1);
    vk::InstanceCreateInfo createInfo({}, &applicationInfo, enabledLayers.size(), enabledLayers.data(),
                                      enabledExtensions.size(), enabledExtensions.data());
    vkInstance = vk::createInstanceUnique(createInfo);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    exit(-1);
  }
  std::cout << "vulkan initialized" << std::endl;
}

void enableRequiredDeviceExtensions(vk::PhysicalDevice device) {
  enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::vector<vk::ExtensionProperties> deviceExtensions = device.enumerateDeviceExtensionProperties();
  std::vector<const char*> deviceExtensionNames(deviceExtensions.size());
  std::transform(deviceExtensions.begin(), deviceExtensions.end(), deviceExtensionNames.begin(),
                 [](const auto &i) -> const char* { return i.extensionName; });

  std::cout << "Supported device extensions:" << std::endl;
  for (const auto &ext : deviceExtensions)
    std::cout << "\t" << ext.extensionName << std::endl;

  auto unsupportedExtensions = enabledDeviceExtensions;
  unsupportedExtensions.erase(std::remove_if(unsupportedExtensions.begin(),
                                             unsupportedExtensions.end(),
                                             [=](const auto &i) {
                                               return
                                                   std::find_if(deviceExtensionNames.begin(),
                                                                deviceExtensionNames.end(),
                                                                [=](const auto &deviceExtensionName) {
                                                                  return !std::strcmp(i, deviceExtensionName);
                                                                }) != deviceExtensionNames.end();
                                             }), unsupportedExtensions.end());
  std::cout << "Unsupported Device Extensions:" << std::endl;
  for (const auto &i : unsupportedExtensions)
    std::cout << "\t" << i << std::endl;
}

void enableRequiredExtensions() {
  std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
  std::cout << "supported extensions:" << std::endl;
  for (const auto &ext : supportedExtensions)
    std::cout << "\t" << ext.extensionName << std::endl;

  std::size_t numExtensions = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(reinterpret_cast<uint32_t *>(&numExtensions));
  for (std::size_t i = 0; i < numExtensions; ++i)
    enabledExtensions.push_back(glfwExtensions[i]);
  std::cout << "GLFW requested the following extensions: " << std::endl;
  for (const auto &ext : enabledExtensions)
    std::cout << "\t" << ext << std::endl;

  for (const auto &reqExt : enabledExtensions) {
    bool found = false;
    for (const auto &ext : supportedExtensions) {
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
  if (DEBUG) enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
  std::vector<vk::LayerProperties> vkLayerProps = vk::enumerateInstanceLayerProperties();
  std::cout << "Supported Layers:" << std::endl;
  for (const auto &layerProps : vkLayerProps)
    std::cout << "\t" << layerProps.layerName << std::endl;
}

void pickDevice() {
  auto physicalDevices = vkInstance->enumeratePhysicalDevices();
  physicalDevices.erase(std::remove_if(physicalDevices.begin(),
                                       physicalDevices.end(),
                                       [](const auto &physicalDevice) {
                                         return physicalDevice.getProperties().deviceType !=
                                                vk::PhysicalDeviceType::eDiscreteGpu;
                                       }),
                        physicalDevices.end());

  physicalDevice = physicalDevices.front();
  std::cout << "Physical Devices: " << std::endl;
  for (auto &device : physicalDevices)
    std::cout << "\t" << device.getProperties().deviceName << std::endl;

  std::vector<vk::QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();
  std::size_t graphicsFamilyIndex = queueFamilyProps.size(), presentFamilyIndex = queueFamilyProps.size();
  for (std::size_t i = 0; i < queueFamilyProps.size(); ++i) {
    bool supportsGraphics = false;
    bool supportsPresenting = false;
    if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
      supportsGraphics = true;
      graphicsFamilyIndex = i;
    }
    if (physicalDevice.getSurfaceSupportKHR(i, uniqueSurface.get())) {
      supportsPresenting = true;
      presentFamilyIndex = i;
    }
    if (supportsGraphics && supportsPresenting) break;
  }
  std::cout << "Queues:" << std::endl;
  for (std::size_t i = 0; i < queueFamilyProps.size(); ++i) {
    std::cout << "\t" << i << std::endl;
    std::cout << "\t\tgraphics: " << static_cast<bool>(queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics) << std::endl;
    std::cout << "\t\tpresentation: " << physicalDevice.getSurfaceSupportKHR(i, uniqueSurface.get()) << std::endl;
  }
  if (graphicsFamilyIndex >= queueFamilyProps.size() || presentFamilyIndex >= queueFamilyProps.size()) {
    std::cerr << "Could not find a queue family supporting graphics and/or presenting." << std::endl;
    cleanup();
    exit(-1);
  }

  enableRequiredDeviceExtensions(physicalDevice);

  std::set<std::size_t> uniqueQueueFamilies = {graphicsFamilyIndex, presentFamilyIndex};
  std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
  std::vector<float*> queuePriorities;
  for(const auto& queueFamily : uniqueQueueFamilies) {
    float priority = 1;
    deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, queueFamily, 1, &priority));
  }

  logicalDevice = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo({}, deviceQueueCreateInfos.size(),
                                                                         deviceQueueCreateInfos.data(),
                                                                         enabledLayers.size(), enabledLayers.data(), enabledDeviceExtensions.size(),
                                                                         enabledDeviceExtensions.data()));

  graphicsQueue = logicalDevice->getQueue(graphicsFamilyIndex, 0);
  presentQueue = logicalDevice->getQueue(presentFamilyIndex, 0);

  std::cout << "Selecting queue " << graphicsFamilyIndex << " for graphics" << std::endl;
  std::cout << "Selecting queue " << presentFamilyIndex << " for presenting" << std::endl;
}

void createSwapChain() {
  swapChainSupportDetails = SwapChainUtils::getSwapChainSupport(physicalDevice, uniqueSurface);
  std::cout << UINT32_MAX << std::endl;
  std::cout << swapChainSupportDetails.capabilities.maxImageExtent.width << std::endl;
  std::cout << swapChainSupportDetails.capabilities.maxImageExtent.height << std::endl;
}

void createWindow() {
  glfwInit();
  window = std::make_unique<Window>(800, 600);
}

void createSurface() {
  auto res = static_cast<vk::Result>(glfwCreateWindowSurface(vkInstance.get(), window->glfwWindow, nullptr,
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
