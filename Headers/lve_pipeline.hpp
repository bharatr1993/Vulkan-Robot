#pragma once

#include "lve_device.hpp"
#include "lve_model.hpp"

// std
#include <string>
#include <vector>
#include <memory>
#include "Structs.h"

namespace lve {

struct PipelineConfigInfo {
  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class LvePipeline {
public:
    LvePipeline(
        LveDevice& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo,
        std::vector<Vertex> vertices,
        std::vector<std::string> filePath,
        std::vector<std::string> objPath,
        std::shared_ptr<LveDescriptorSet> descSet,
        bool indirect);

    LvePipeline(
        LveDevice& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo,
        std::vector<std::string> filePath,
        std::vector<std::string> objPath,
        std::shared_ptr<LveDescriptorSet> descSet,
        bool indirect,
        bool particleSystem);

    ~LvePipeline();

    LvePipeline(const LvePipeline&) = delete;
    void operator=(const LvePipeline&) = delete;

    static constexpr bool use_index_buffer = true;

    void bind(VkCommandBuffer commandBuffer);

    static void defaultPipelineConfigInfo(
        PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);

    std::vector<std::shared_ptr<LveModel>> lveModel;

    void createPipelineLayout(std::shared_ptr<LveDescriptorSet> descriptorSet);

    VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    void DescriptorbindPipeline(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet> DescriptorSets){
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, DescriptorSets.size(), DescriptorSets.data(), 0, (uint32_t*)nullptr);
    }

    void loadModels(std::vector<Vertex> vertices, std::vector<uint32_t> index, std::vector<std::string> filePath);
    void loadModels(std::vector<Vertex> vertices, std::vector<std::string> filePath);
    void loadModels(std::vector<std::string> objPath, std::vector<std::string> filePath);
    void loadModelsIndirect(std::vector<std::string> objPath, std::vector<std::string> filePath, int drawCount);
    void updateIndirectCmdBuffer(int drawCount);

    std::vector<VkVertexInputBindingDescription> getBindingDescriptions(bool indirect);
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(bool indirect);
    VkPipeline graphicsPipeline;

    bool indirectDraw;
    bool particlePipeline;
 private:
  static std::vector<char> readFile(const std::string& filepath);

  void createGraphicsPipeline(
      const std::string& vertFilepath,
      const std::string& fragFilepath,
      const PipelineConfigInfo& configInfo,
      bool indirect);


  void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

  LveDevice& lveDevice;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineLayout pipelineLayout;
  std::shared_ptr<LveDescriptorSet> descriptorSet;


};
}  // namespace lve