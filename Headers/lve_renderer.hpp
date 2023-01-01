#pragma once
#include <vector>
#include <string>
#include "lve_pipeline.hpp"
#include "lve_computepipeline.hpp"
#include "lve_model.hpp"
#include "lve_device.hpp"
#include "lve_window.hpp"
#include "lve_swap_chain.hpp"
#include "lve_buffer.hpp"
#include <memory>
#include <array>
#include "Structs.h"

static constexpr int no_tf_matrices = 1;

namespace lve {
	class LveRenderer {
	public:
		LveRenderer(LveDevice &lDevice,
                    LveWindow& lWindow,
                    LveSwapChain& lSwapChain,
                    std::vector<std::string> vPath,
                    std::vector<std::string> fPath,
                    std::vector<std::string> tPath,
                    std::vector<std::vector<std::string>> objPath);

        LveRenderer(LveDevice &lDevice,
                    LveWindow& lWindow,
                    LveSwapChain& lSwapChain,
                    std::vector<std::string> vPath,
                    std::vector<std::string> fPath,
                    std::string compPath,
                    std::vector<std::string> tPath,
                    std::vector<Vertex> vertices,
                    std::vector<std::vector<std::string>> objPath);
		~LveRenderer();

		void initCommandBuffers();
		void createPipeline();
		void createCommandBuffers(std::vector<std::string> pipelineNames);
		void drawFrame();

		void createPoolSize();
		void createLayoutBindingSize(size_t size);
        void createComputeCommandBuffer();

        void initParticleBuffer();
        void prepareParticleBuffer();

        std::unordered_map<std::string, uint32_t> pipelineMap={};
		LveDevice& lveDevice;
		LveWindow& lveWindow;
		LveSwapChain& lveSwapChain;

		std::vector<std::unique_ptr<LvePipeline>> lveVecPipeline;

		std::vector<std::vector<std::string>> objectPath;

		
		std::shared_ptr<LveDescriptorSet> descriptorSet;
		std::vector<std::shared_ptr<LveBuffer>> lveBufferPtrVec;

		Transform transform = {};
		colorTF tr = { {0.0f,1.0f,1.0f, 1.0f} };
        std::unique_ptr<LveComputePipeline> lveComputePipeline;


	private:
		std::vector<std::string> vertPath, fragPath, texPath;
        std::string computePath;
		std::vector<VkCommandBuffer> commandBuffers;
        VkCommandBuffer computeCommandBuffers;
		std::vector<VkDescriptorPoolSize> poolSize;
		std::vector<VkDescriptorSetLayoutBinding> binding;
        std::vector<Vertex> vertex;
        int pipelineCount=0;
        int pipelineCountPar=0;
        QueueFamilyIndices indices;
	};
}