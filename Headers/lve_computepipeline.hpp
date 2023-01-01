//
// Created by bhara on 02-12-2022.
//

#pragma once

#include "lve_device.hpp"
#include "lve_model.hpp"

// std
#include <string>
#include <vector>
#include <memory>
#include "Structs.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace lve{
    class LveComputePipeline{

        LveDevice& lveDevice;
        VkShaderModule computeShaderModule;

        static std::vector<char> readFile(const std::string& filepath);

        VkDeviceMemory ParticleBufferMemory, stagingParticleBufferMemory;

    public:
        LveComputePipeline(
                LveDevice& device,
                std::string compFilepath,
                std::shared_ptr<LveDescriptorSet> descSet);

        ~LveComputePipeline();

        void prepareMiscBuffer(uint32_t count, float fElapsedTime);
        VkBuffer miscBuffer;
        VkDeviceMemory miscBufferMemory;

        std::shared_ptr<LveDescriptorSet> descriptorSet;
        VkPipelineLayout computePipelineLayout;
        VkPipeline computePipeline;
        miscparticleinfo miscInfo;

        LveComputePipeline(const LveComputePipeline&) = delete;
        void operator=(const LveComputePipeline&) = delete;

        void createComputePipeline(const std::string& computeFilepath);
        void createComputePipelineLayout(const std::string& computeFilepath);

        void updateComputeDescriptor();

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        void initParticleBuffer();
        void prepareParticleBuffer();

        void commitMiscBuffer(double fElapsedTime);

        std::vector<particlebuf> particlebuffer;
        VkBuffer ParticleBuffer, stagingParticleBuffer;

        uint32_t particleBufferSize;
    };
}
