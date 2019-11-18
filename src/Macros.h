#ifndef VULKAN_MACROS_H
#define VULKAN_MACROS_H

#include <filesystem>

namespace fs = std::filesystem;

#ifndef NDEBUG
#define DEBUG
#endif

// To avoid a win32 macro
#undef max

template<class T>
constexpr uint32_t uint32(T t) {
  return static_cast<uint32_t>(t);
}

#endif
