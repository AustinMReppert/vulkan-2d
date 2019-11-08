#ifndef VULKAN_MACROS_H
#define VULKAN_MACROS_H

#include <filesystem>

#define PS std::filesystem::path::preferred_separator
namespace fs = std::filesystem;

#ifndef NDEBUG
#define DEBUG
#endif

template<class T>
constexpr uint32_t uint32(T t) {
  return static_cast<uint32_t>(t);
}

#endif
