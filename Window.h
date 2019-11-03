#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

#include <string_view>

class Window {

public:
  Window();

  Window(uint32_t width, uint32_t height);

  ~Window();

  GLFWwindow *glfwWindow;

  uint32_t getWidth();

  uint32_t getHeight();

  void center();

  void setTitle(const std::string_view& title);

  void zoom();

  void minimize();

};

#endif
