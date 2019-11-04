#include "main.h"
#include "DeviceUtils.h"
#include "ShaderUtils.h"

#ifndef NDEBUG
#define DEBUG
#endif

void initVk() {
  try {
    vk::ApplicationInfo applicationInfo("vulkan", vk::makeVersion(1, 0, 0), "", vk::makeVersion(1, 0, 0),
                                        vk::makeVersion(1, 0, 0));
    vk::InstanceCreateInfo createInfo({}, &applicationInfo, vk::size(enabledLayers), enabledLayers.data(),
                                      vk::size(enabledExtensions), enabledExtensions.data());
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

  if (!unsupportedExtensions.empty()) {
    std::cerr << "Missing Device Extensions:" << std::endl;
    for (const auto& i : unsupportedExtensions)
      std::cerr << "\t" << i << std::endl;
    exit(-1);
  }
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
  physicalDevice = DeviceUtils::getOptimalPhysicalDevice();

#ifdef DEBUG
  std::cout << "Physical Devices: " << std::endl;
  for (auto& device : physicalDevices)
    std::cout << "\t" << device.getProperties().deviceName << std::endl;
#endif

  std::set<std::optional<uint32_t>> queueFamilyIndices = DeviceUtils::getGraphicsAndPresentQueueFamilyIndices(
      physicalDevice,
      uniqueSurface);


#ifdef DEBUG
  std::vector<vk::QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();
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

  for (const std::optional<uint32_t>& queueFamilyIndex : queueFamilyIndices) {
    if (queueFamilyIndex.has_value()) continue;
    std::cerr << "Could not find a queue family supporting graphics and/or presenting." << std::endl;
    cleanup();
    exit(-1);
  }

  graphicsQueueFamilyIndex = queueFamilyIndices.begin()->value();
  presentQueueFamilyIndex = queueFamilyIndices.rbegin()->value();

  enableRequiredDeviceExtensions(physicalDevice);
  std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
  std::vector<float *> queuePriorities;
  for (const auto& queueFamilyIndex : queueFamilyIndices) {
    float priority = 1;
    deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, queueFamilyIndex.value(), 1, &priority));
  }

  logicalDevice = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo({}, vk::size(deviceQueueCreateInfos),
                                                                         deviceQueueCreateInfos.data(),
                                                                         vk::size(enabledLayers), enabledLayers.data(),
                                                                         vk::size(enabledDeviceExtensions),
                                                                         enabledDeviceExtensions.data()));

  graphicsQueue = logicalDevice->getQueue(graphicsQueueFamilyIndex, 0);
  presentQueue = logicalDevice->getQueue(presentQueueFamilyIndex, 0);

#ifdef DEBUG
  std::cout << "Selecting queue " << graphicsQueueFamilyIndex << " for graphics" << std::endl;
  std::cout << "Selecting queue " << presentQueueFamilyIndex << " for presenting" << std::endl;
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
  window->center();
  window->setTitle("Vulkan");
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

void createShaders() {
  std::unique_ptr<std::string> vertexSrc = ShaderUtils::load("..\\shaders\\vertex.vert");
  std::unique_ptr<std::string> fragmentSrc = ShaderUtils::load("..\\shaders\\fragment.frag");

  std::unique_ptr<std::string> vertexShaderPreprocessed = ShaderUtils::preprocess("vertex.vert", *vertexSrc,
                                                                          shaderc_shader_kind::shaderc_vertex_shader);
  std::unique_ptr<std::string> fragmentShaderPreprocessed = ShaderUtils::preprocess("fragment.frag", *fragmentSrc,
                                                                          shaderc_shader_kind::shaderc_fragment_shader);

  std::unique_ptr<std::vector<uint32_t>> vertexSpvByteCode = ShaderUtils::compile("vertex.vert", *vertexShaderPreprocessed,
                                                                            shaderc_shader_kind::shaderc_vertex_shader, false);
  std::unique_ptr<std::vector<uint32_t>> fragmentSpvByteCode = ShaderUtils::compile("fragment.frag", *fragmentShaderPreprocessed,
                                                                            shaderc_shader_kind::shaderc_fragment_shader, false);

  vertShaderModUnique = ShaderUtils::createShader(logicalDevice, *vertexSpvByteCode);
  vertShaderModUnique = ShaderUtils::createShader(logicalDevice, *fragmentSpvByteCode);

}

int main() {

  createWindow();

  enableRequiredExtensions();
  enableRequiredLayers();
  initVk();
  createSurface();
  pickDevice();
  createSwapChain();
  createShaders();
  while (!glfwWindowShouldClose(window->glfwWindow)) {
    glfwPollEvents();
  }

  cleanup();
  return 0;
}
