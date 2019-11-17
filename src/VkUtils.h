#ifndef VULKAN_VKUTILS_H
#define VULKAN_VKUTILS_H

#include <cstdint>

namespace vk {
  constexpr uint32_t makeVersion(uint32_t major, uint32_t minor, uint32_t patch) {
    return major << static_cast<uint32_t>(22) | (minor << static_cast<uint32_t>(12)) | (patch);
  }

  template<class T>
  constexpr uint32_t size(T t) {
    return static_cast<uint32_t>(t.size());
  }

}

#endif
