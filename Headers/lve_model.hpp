#pragma once

#include "lve_device.hpp"
#include "lve_descriptors.hpp"
#include "lve_buffer.hpp"
#include "Structs.h"
#include "lve_texture.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"


// std
#include <vector>

namespace lve {

	class LveModel {
	public:


		LveModel(LveDevice& device, const std::vector<Vertex>& vertices, std::vector<std::string> filePath, std::shared_ptr<LveDescriptorSet> descSet);
		LveModel(LveDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& index, std::vector<std::string> filePath, std::shared_ptr<LveDescriptorSet> descSet);
		LveModel(LveDevice& device, std::string objPath, std::vector<std::string> filePath, std::shared_ptr<LveDescriptorSet> descSet, bool drawIndirect);
		~LveModel();

		LveModel(const LveModel&) = delete;
		LveModel& operator=(const LveModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void indexbind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		void indexdraw(VkCommandBuffer commandBuffer);
        void indirectDraw(VkCommandBuffer commandBuffer);
        void updateIndirectBuffer(int drawCount);
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::shared_ptr<LveDescriptorSet> descriptorSet;
        void createVertexBuffersIndirect(const std::vector<Vertex>& vertices, int drawCount);
        bool indirect;
        VkBuffer vertexBuffer, indexBuffer, stagingVertexBuffer, indDrawCmdBuffer;
	private:
		
		void createVertexBuffers(const std::vector<Vertex>& vertices);

		void createIndexBuffers(const std::vector<uint32_t>& index);
		


		LveDevice& lveDevice;

		VkDeviceMemory vertexBufferMemory, indexBufferMemory, stagingVertexBufferMemory, indDrawCmdMemory;
		uint32_t vertexCount, indexCount;
        std::vector<VkDrawIndirectCommand> indirectCommands;

		
	};
}  // namespace lve
