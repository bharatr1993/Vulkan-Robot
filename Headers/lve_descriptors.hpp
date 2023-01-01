#pragma once

#include "lve_device.hpp"
#include "lve_texture.hpp"
#include "lve_buffer.hpp"
// std
#include <memory>
#include <unordered_map>
#include <vector>
#include "Structs.h"

namespace lve {

    class LveDescriptorSet {
    public:

        LveDescriptorSet(
            LveDevice& lveDevice, std::vector<VkDescriptorPoolSize> pSizes, std::vector<VkDescriptorSetLayoutBinding> bindings);
        ~LveDescriptorSet();

        std::vector<VkDescriptorSetLayout> getDescriptorSetLayout() const { return descriptorSetLayout; }
        std::vector<VkDescriptorSetLayout> getComputeDescriptorSetLayout() const { return computeDescriptorSetLayout; }
        std::vector<VkDescriptorSet> descriptor;
        VkDescriptorSet computeDescriptor[2];
        std::vector<VkDescriptorSetLayoutBinding> binding;
        VkDescriptorSetLayoutBinding computeBinding[2];
        std::vector<VkDescriptorSetLayout> descriptorSetLayout;
        std::vector<VkDescriptorSetLayout> computeDescriptorSetLayout;
        void writeBufferReference(std::vector<std::shared_ptr<LveBuffer>> buffer) { buffers = buffer; }//Later change to vector and modify lveModel.cpp
        void writeDescriptorSet();
        std::unique_ptr<LveTexture> lveTexture;
        void updateTransformDescriptorSet();

    private:
        LveDevice& lveDevice;
        std::vector<VkWriteDescriptorSet> writeDscSet;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
        std::vector<std::shared_ptr<LveBuffer>> buffers; //Later change to vector and modify lveModel.cpp

        bool allocateDescriptor();

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

        void createPool();

        VkDescriptorPool descPool, computeDescPool;

        void overwrite(VkDescriptorSet& set);



        std::vector<VkWriteDescriptorSet> writes;




    };

}  // namespace lve
