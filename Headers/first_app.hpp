#pragma once

#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"
#include "lve_transform.hpp"
#include "lve_renderer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include <chrono>
#include "Structs.h"
// std
#include <memory>
#include <vector>
#include <string.h>
#include <iostream>
#include <math.h>


namespace lve {
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:

double clamp(double x, double upper, double lower)
{
    return std::min(upper, std::max(x, lower));
}

  void renderScene();
  std::vector<std::string> vertPath = {  {"../Shaders/cabin_vert.spv"}, {"../Shaders/robot_vert.spv"}, {"../Shaders/container_vert.spv"} , {"../Shaders/legleft_vert.spv"}, {"../Shaders/legright_vert.spv"}};
  std::vector<std::string> fragPath = { {"../Shaders/cabin_frag.spv"}, {"../Shaders/robot_frag.spv"}, {"../Shaders/container_frag.spv"}, {"../Shaders/legleft_frag.spv"}, {"../Shaders/legright_frag.spv"}};
  std::vector<std::string> bmpPath = { {"../Pictures/robot.bmp"},{"../Pictures/container.bmp"}, {"../Pictures/pula.bmp"},{"../Pictures/fireem.bmp"}, {"../Pictures/ball2.bmp"}};
  std::string compPath = "../Shaders/pula_comp.spv"; //Unused----Testing
  std::vector<std::vector<std::string>> objPath = {
                                                    {"../Pictures/pula.obj"},
                                                    {"../Pictures/robotbody.obj"},
                                                    {"../Pictures/container.obj"},
                                                    {"../Pictures/legleft.obj"},
                                                    {"../Pictures/legright.obj"}
                                                    };
  std::vector<std::string> pipelineNames = {};
  TransformComponent tfMat={}, bMat={},legLeftMat={},legRightMat={}, cameraMat={}, cabinMat={};
  std::chrono::high_resolution_clock::time_point tstart;
  std::unique_ptr<LveRenderer> lveRenderer;
  double fElapsedTime=0;
  bool init = true;
  float time = 0;

    std::vector<Vertex> vertices = {};

    //IGNORE----------------------------------------------------------
  /*
      //{ {{-0.5f, -0.5f}}, {{-0.5f, 0.5f}}, {{0.5f, 0.5f}} }, { {{0.0f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}} } };
    {
    {{-0.5f, -0.5f, +0.5f},
    {  0.,  0.,  1. },
    {  0.,  0.,  1. },
    {  0., 1. }},
    {{+0.5f, -0.5f, +0.5f},
    {  0.,  0.,  1. },
    {  1.,  0.,  1. },
    {  1., 1. }},
    {{+0.5f, +0.5f, +0.5f},
    {  0.,  0.,  1. },
    {  1.,  1.,  1. },
    {  1., 0. }},
    {{-0.5f, +0.5f, +0.5f},
    {  0.,  0.,  1. },
    {  0.,  1.,  1. },
    {  0., 0. }},
    {{-0.5f, -0.5f, -0.5f},
    {  0.,  0., -1. },
    {  0.,  0.,  0. },
    {  1., 0. }},
    {{+0.5f, -0.5f, -0.5f},
    {  0.,  0., -1. },
    {  1.,  0.,  0. },
    {  0., 0. }},
    {{+0.5f, +0.5f, -0.5f},
    {  0.,  0., -1. },
    {  1.,  1.,  0. },
    {  0., 1. }},
    {{-0.5f, +0.5f, -0.5f},
    { -1.,  0.,  0. },
    {  0.,  1.,  0. },
    {  0., 1. }}
    }
  };

  //--- 0
  //--+ 1
  //-++ 2
  //++- 3
  //+-+ 4
  //+++ 5
  //+-- 6
  //-+- 7
  //------------------------------------------------------------

  std::vector<uint32_t> index = {
        0, 1, 2,
        3, 0, 2,

        1, 5, 6,
        6, 2, 1,

        7, 6, 5,
        5, 4, 7,

        4, 0, 3,
        3, 7, 4,

        4, 5, 1,
        1, 0, 4,

        3, 2, 6,
        6, 7, 3
    */


  LveWindow lveWindow{WIDTH, HEIGHT, "Vulkan: Scarlet Storm!"};
  LveDevice lveDevice{lveWindow};
  LveSwapChain lveSwapChain{lveDevice, lveWindow.getExtent()};
};
}  // namespace lve