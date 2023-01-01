//
// Created by bhara on 02-12-2022.
//
#include "lve_computepipeline.hpp"

namespace lve{



    LveComputePipeline::LveComputePipeline(LveDevice& device,std::string compFilepath,std::shared_ptr<LveDescriptorSet> descSet): lveDevice{device}, descriptorSet{descSet}{
        initParticleBuffer();
        prepareParticleBuffer();
        createComputePipelineLayout(compFilepath);
        createComputePipeline(compFilepath);

    }

    LveComputePipeline::~LveComputePipeline(){
        vkDestroyShaderModule(lveDevice.device(), computeShaderModule, nullptr);
        vkFreeMemory(lveDevice.device(), miscBufferMemory, nullptr);
        vkDestroyBuffer(lveDevice.device(), miscBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), ParticleBufferMemory, nullptr);
        vkDestroyBuffer(lveDevice.device(), ParticleBuffer, nullptr);
    }

    void LveComputePipeline::initParticleBuffer() {
        particlebuf pushparticle={};
        for (int i=0;i<100;i++) {
            float x = ((rand()%100)/100.0f);
            float y = ((rand()%100)/100.0f);
            //float x = 0.5f,y=-0.5f;

            pushparticle.position=glm::vec2(x,y);
            pushparticle.velocity=glm::vec2(0.1f);
            pushparticle.radius =glm::vec2(1.25f*((rand()%100)/100.0f));
            pushparticle.offset =glm::vec2(((rand()%200)/100.0f) - 1., ((rand()%200)/100.0f) - 1.0f);
            //pushparticle.totalcount=100;
            particlebuffer.push_back(pushparticle);

        }
    }


    void LveComputePipeline::prepareParticleBuffer() {
        particleBufferSize=particlebuffer.size()*sizeof(particlebuf);
        lveDevice.createBuffer(
                particleBufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingParticleBuffer,
                stagingParticleBufferMemory);

        lveDevice.createBuffer(
                particleBufferSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                ParticleBuffer,
                ParticleBufferMemory);

        void *data;

        vkMapMemory(lveDevice.device(), stagingParticleBufferMemory, 0, particleBufferSize, 0, &data);
        memcpy(data, particlebuffer.data(), static_cast<size_t>(particleBufferSize));
        vkUnmapMemory(lveDevice.device(), stagingParticleBufferMemory);

        lveDevice.copyBuffer(stagingParticleBuffer,ParticleBuffer,static_cast<VkDeviceSize>(particleBufferSize));

        vkFreeMemory(lveDevice.device(), stagingParticleBufferMemory, nullptr);
        vkDestroyBuffer(lveDevice.device(), stagingParticleBuffer, nullptr);
    }


    void LveComputePipeline::updateComputeDescriptor() {
        //Write to Descriptor
        VkWriteDescriptorSet vwds0;
        VkDescriptorBufferInfo vdbi0;

        vdbi0.range = sizeof(particlebuf)*particlebuffer.size();
        vdbi0.buffer = ParticleBuffer;//Change for vec impl
        vdbi0.offset = 0;	// bytes
        vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vwds0.pNext = nullptr;
        vwds0.dstSet = descriptorSet->computeDescriptor[0];
        vwds0.dstBinding = 0;
        vwds0.dstArrayElement = 0;
        vwds0.descriptorCount = 1;
        vwds0.pTexelBufferView = (VkBufferView*)nullptr;
        vwds0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        vwds0.pBufferInfo = &vdbi0;
        vwds0.pImageInfo = (VkDescriptorImageInfo*)nullptr;

        vkUpdateDescriptorSets(lveDevice.device(), 1, &vwds0, 0, (VkCopyDescriptorSet*)nullptr);

    }

    void LveComputePipeline::commitMiscBuffer(double fElapsedTime) {
        uint32_t bufferSize=sizeof(miscInfo);
        miscInfo.fElapsedTime.x=fElapsedTime;
        miscInfo.totalcount.x=100;

        void* data;
        vkMapMemory(lveDevice.device(), miscBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, &miscInfo, static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), miscBufferMemory);

        VkWriteDescriptorSet vwds0;
        VkDescriptorBufferInfo vdbi0;

        vdbi0.range = sizeof(miscInfo);
        vdbi0.buffer = miscBuffer;//Change for vec impl
        vdbi0.offset = 0;	// bytes
        vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vwds0.pNext = nullptr;
        vwds0.dstSet = descriptorSet->computeDescriptor[1];
        vwds0.dstBinding = 0;
        vwds0.dstArrayElement = 0;
        vwds0.descriptorCount = 1;
        vwds0.pTexelBufferView = (VkBufferView*)nullptr;
        vwds0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vwds0.pBufferInfo = &vdbi0;
        vwds0.pImageInfo = (VkDescriptorImageInfo*)nullptr;

        vkUpdateDescriptorSets(lveDevice.device(), 1, &vwds0, 0, (VkCopyDescriptorSet*)nullptr);

    }

    void LveComputePipeline::prepareMiscBuffer(uint32_t count, float fElapsedTime) {
        miscInfo.totalcount.x=count;
        miscInfo.fElapsedTime.x=fElapsedTime;
        uint32_t bufferSize=sizeof(miscInfo);
        lveDevice.createBuffer(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                miscBuffer,
                miscBufferMemory);

        void *data;

        vkMapMemory(lveDevice.device(), miscBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, &miscInfo, static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), miscBufferMemory);
    }

    void LveComputePipeline::createComputePipelineLayout(const std::string &computeFilepath) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        std::vector<VkDescriptorSetLayout> computeDescriptorLayout = descriptorSet->getComputeDescriptorSetLayout();
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 2;
        pipelineLayoutInfo.pSetLayouts = computeDescriptorLayout.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline layout!");
        }
    }

    std::vector<char> LveComputePipeline::readFile(const std::string& filepath) {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void LveComputePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute shader module");
        }
    }

    void LveComputePipeline::createComputePipeline(const std::string &computeFilepath) {
        auto computeCode = readFile(computeFilepath);
        createShaderModule(computeCode, &computeShaderModule);

        VkPipelineShaderStageCreateInfo shaderStagesCompute;
        shaderStagesCompute.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStagesCompute.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStagesCompute.module = computeShaderModule;
        shaderStagesCompute.pName = "main";
        shaderStagesCompute.flags = 0;
        shaderStagesCompute.pNext = nullptr;
        shaderStagesCompute.pSpecializationInfo = nullptr;

        VkComputePipelineCreateInfo computePipelineInfo{};
        computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineInfo.stage = shaderStagesCompute;
        computePipelineInfo.layout = computePipelineLayout;

        if (vkCreateComputePipelines(lveDevice.device(),VK_NULL_HANDLE,1,&computePipelineInfo,nullptr,&computePipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline");
        }
    }
}