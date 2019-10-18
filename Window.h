#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

class Window {

 public:
  Window();
  Window(int width, int height);
  ~Window();
  GLFWwindow* glfwWindow;
  int getWidth();
  int getHeight();
};

#endif
