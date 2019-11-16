#include "Renderer.h"

#include <filesystem>

#include <shaderc/shaderc.hpp>

#include "Macros.h"
#include "DeviceUtils.h"
#include "ShaderUtils.h"

void Renderer::initVk() {
  try {
    vk::ApplicationInfo applicationInfo("vulkan", vk::makeVersion(1, 0, 0), "", vk::makeVersion(1, 0, 0),
                                        vk::makeVersion(1, 0, 0));
    vk::InstanceCreateInfo createInfo({}, &applicationInfo, vk::size(enabledLayers), enabledLayers.data(),
                                      vk::size(enabledExtensions), enabledExtensions.data());
    vkInstance = vk::createInstanceUnique(createInfo);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    exit(-1);
  }

#ifdef DEBUG
  std::cout << "vulkan initialized" << std::endl;
#endif

}

void Renderer::enableRequiredDeviceExtensions(vk::PhysicalDevice device) {
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

void Renderer::enableRequiredExtensions() {
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

void Renderer::enableRequiredLayers() {
  std::vector<vk::LayerProperties> vkLayerProps = vk::enumerateInstanceLayerProperties();

#ifdef DEBUG
  enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
  std::cout << "Supported Layers:" << std::endl;
  for (const auto& layerProps : vkLayerProps)
    std::cout << "\t" << layerProps.layerName << std::endl;
#endif

}

void Renderer::pickDevice() {
  auto physicalDevices = vkInstance->enumeratePhysicalDevices();
  physicalDevice = DeviceUtils::getOptimalPhysicalDevice(vkInstance);

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
              << static_cast<bool>(queueFamilyProps[uint32(i)].queueFlags & vk::QueueFlagBits::eGraphics)
              << std::endl;
    std::cout << "\tpresentation: "
              << physicalDevice.getSurfaceSupportKHR(uint32(i), uniqueSurface.get()) << std::endl;
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

  try {
    logicalDevice = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo({}, vk::size(deviceQueueCreateInfos),
                                                                           deviceQueueCreateInfos.data(),
                                                                           vk::size(enabledLayers),
                                                                           enabledLayers.data(),
                                                                           vk::size(enabledDeviceExtensions),
                                                                           enabledDeviceExtensions.data()));
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    cleanup();
    exit(-1);
  }

  graphicsQueue = logicalDevice->getQueue(graphicsQueueFamilyIndex, 0);
  presentQueue = logicalDevice->getQueue(presentQueueFamilyIndex, 0);

#ifdef DEBUG
  std::cout << "Selecting queue " << graphicsQueueFamilyIndex << " for graphics" << std::endl;
  std::cout << "Selecting queue " << presentQueueFamilyIndex << " for presenting" << std::endl;
#endif

}

void Renderer::createSwapChain() {
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

void Renderer::createWindow() {
  glfwInit();
  window = std::make_unique<Window>(800, 600);
  window->center();
  window->setTitle("Vulkan");
}

void Renderer::createSurface() {
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

void Renderer::cleanup() {
  glfwTerminate();
}

void Renderer::createShaders() {
  try {
    vertShaderModUnique = ShaderUtils::createShader(logicalDevice, fs::current_path().parent_path().append(
        "shaders").append("vertex.vert"), shaderc_shader_kind::shaderc_vertex_shader);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }
  try {
    fragShaderModUnique = ShaderUtils::createShader(logicalDevice, fs::current_path().parent_path().append(
        "shaders").append("fragment.frag"), shaderc_shader_kind::shaderc_fragment_shader);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    cleanup();
    exit(1);
  }

}

void Renderer::createPipeline() {
  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = {
      vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShaderModUnique.get(), "main",
                                        nullptr},
      {{}, vk::ShaderStageFlagBits::eFragment, fragShaderModUnique.get(), "main", nullptr}};

  vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {{}, 0, nullptr, 0, nullptr};
  vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo = {{}, vk::PrimitiveTopology::eTriangleList,
                                                                      VK_FALSE};
  vk::Viewport viewport = {0, 0, static_cast<float>(optimalExtent.width), static_cast<float>(optimalExtent.height), 0,
                           1};
  vk::Rect2D scissor = {{0, 0}, optimalExtent};
  vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {{}, 1, &viewport, 1, &scissor};
  vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {{}, VK_FALSE, VK_FALSE,
                                                                                   vk::PolygonMode::eFill,
                                                                                   vk::CullModeFlagBits::eBack,
                                                                                   vk::FrontFace::eClockwise, 0, 0, 0,
                                                                                   0, 1};
  vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {{}, vk::SampleCountFlagBits::e1,
                                                                               VK_FALSE, 0, nullptr, VK_FALSE,
                                                                               VK_FALSE};
  vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = {VK_FALSE, vk::BlendFactor::eOne,
                                                                     vk::BlendFactor::eZero, vk::BlendOp::eAdd,
                                                                     vk::BlendFactor::eOne, vk::BlendFactor::eZero,
                                                                     vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR |
                                                                                        vk::ColorComponentFlagBits::eG |
                                                                                        vk::ColorComponentFlagBits::eB |
                                                                                        vk::ColorComponentFlagBits::eA};
  std::array<float, 4> blendConstants = {0, 0, 0, 0};
  vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {{}, VK_FALSE, vk::LogicOp::eCopy, 1,
                                                                     &colorBlendAttachmentState, blendConstants};
  vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {{}, 0, nullptr, 0, nullptr};
  pipelineLayoutUnique = logicalDevice->createPipelineLayoutUnique(pipelineLayoutCreateInfo);

#ifdef DEBUG
  std::cout << "Fixed Function Pipeline setup" << std::endl;
#endif

  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {{}, vk::size(shaderStageCreateInfos),
                                                               shaderStageCreateInfos.data(),
                                                               &vertexInputStateCreateInfo, &assemblyStateCreateInfo,
                                                               nullptr, &viewportStateCreateInfo,
                                                               &pipelineRasterizationStateCreateInfo,
                                                               &pipelineMultisampleStateCreateInfo, nullptr,
                                                               &colorBlendStateCreateInfo, nullptr,
                                                               pipelineLayoutUnique.get(), renderPassUnique.get(), 0,
                                                               nullptr, -1};

  try {
    graphicsPipeLineUnique = logicalDevice->createGraphicsPipelineUnique(nullptr, graphicsPipelineCreateInfo);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    cleanup();
  }

#ifdef DEBUG
  std::cout << "Graphics Pipeline created" << std::endl;
#endif

}

void Renderer::createRenderPass() {
  vk::AttachmentDescription colorAttachment = {{}, optimalSurfaceFormat.format, vk::SampleCountFlagBits::e1,
                                               vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                               vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                               vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR};
  vk::AttachmentReference attachmentReference = {0, vk::ImageLayout::eColorAttachmentOptimal};
  vk::SubpassDescription subpass = {{}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &attachmentReference,
                                    nullptr,
                                    nullptr, 0, nullptr};
  vk::RenderPassCreateInfo renderPassCreateInfo = {{}, 1, &colorAttachment, 1, &subpass, 0, nullptr};
  try {
    renderPassUnique = logicalDevice->createRenderPassUnique(renderPassCreateInfo);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    cleanup();
    exit(1);
  }
#ifdef DEBUG
  std::cout << "Render Pass created" << std::endl;
#endif
}
