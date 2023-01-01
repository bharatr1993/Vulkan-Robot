#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include "Structs.h"

#include <string>
namespace lve {
class LveWindow{
 public:
  LveWindow(int w, int h, std::string name);
  ~LveWindow();

  LveWindow(const LveWindow &) = delete;
  LveWindow &operator=(const LveWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

  float getAspectRatio() { return ((float)width / (float)height); }

  GLFWwindow *window;
 private:
  void initWindow();

  const int width;
  const int height;

  std::string windowName;

};

}  // namespace lve