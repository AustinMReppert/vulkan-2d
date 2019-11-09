#ifndef VULKAN_SHADERUTILS_H
#define VULKAN_SHADERUTILS_H

#include <shaderc/shaderc.hpp>

#include <fstream>
#include <string_view>
#include <memory>
#include <string>
#include <filesystem>

class ShaderUtils {

public:
  static std::unique_ptr<std::string> load(const std::string_view& fileName) {
    std::ifstream file(fileName.data(), std::ios_base::ate | std::ios_base::binary);
    std::size_t fileSize = static_cast<std::size_t>(file.tellg());
    file.seekg(0);
    auto rawSrc = new std::vector<char>(fileSize + 1);
    file.read(rawSrc->data(), fileSize);
    file.close();
    (*rawSrc)[fileSize] = '\0';
    std::unique_ptr<std::string> src = std::make_unique<std::string>(std::string(rawSrc->data()));
    delete rawSrc;
    return src;
  }

  static std::unique_ptr<std::string>
  preprocess(const std::string& srcName, const std::string& src, shaderc_shader_kind kind) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions compilerOptions;
    shaderc::PreprocessedSourceCompilationResult res = compiler.PreprocessGlsl(src, kind, srcName.c_str(),
                                                                               compilerOptions);
    if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
      std::cerr << res.GetErrorMessage() << std::endl;
      return std::make_unique<std::string>(nullptr);
    }
    return std::make_unique<std::string>(std::string{res.begin(), res.end()});
  }

  static std::unique_ptr<std::vector<uint32_t>>
  compile(const std::string& srcName, const std::string& src, shaderc_shader_kind kind, bool optimize = false) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions compilerOptions;
    if (optimize) compilerOptions.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult res = compiler.CompileGlslToSpv(src, kind, srcName.c_str(),
                                                                  compilerOptions);
    if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
      std::cerr << res.GetErrorMessage() << std::endl;
      return {};
    }
    return std::make_unique<std::vector<uint32_t>>(std::vector<uint32_t>{res.begin(), res.end()});
  }

  static vk::UniqueShaderModule createShader(const vk::UniqueDevice& device, const std::vector<uint32_t>& spvByteCode) {
    vk::ShaderModuleCreateInfo createInfo = {vk::ShaderModuleCreateFlags(), spvByteCode.size() * sizeof(uint32_t),
                                             spvByteCode.data()};
    return device->createShaderModuleUnique(createInfo);
  }

  static vk::UniqueShaderModule
  createShader(const vk::UniqueDevice& device, const fs::path& path, const shaderc_shader_kind shaderKind,
               bool optimize = false) {
    if(!fs::exists(path)) {
      std::string message = path.string() + " not found";
      throw std::exception(message.c_str());
    }
    std::unique_ptr<std::string> src = ShaderUtils::load(path.string());

    std::unique_ptr<std::string> shaderPreprocessed = ShaderUtils::preprocess(path.filename().string(), *src,
                                                                              shaderKind);

    std::unique_ptr<std::vector<uint32_t>> spvByteCode = ShaderUtils::compile(path.filename().string(),
                                                                              *shaderPreprocessed,
                                                                              shaderKind, optimize);

    return ShaderUtils::createShader(device, *spvByteCode);
  }

protected:
private:

};

#endif
