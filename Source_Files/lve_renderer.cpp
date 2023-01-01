#include "lve_renderer.hpp"

namespace lve {

    LveRenderer::LveRenderer(LveDevice& lDevice,
                             LveWindow& lWindow,
                             LveSwapChain& lSwapChain,
                             std::vector<std::string> vPath,
                             std::vector<std::string> fPath,
                             std::vector<std::string> tPath,
                             std::vector<std::vector<std::string>> objPath)
        : lveDevice{ lDevice }, lveWindow{ lWindow }, lveSwapChain{ lSwapChain }, objectPath{ objPath }, vertPath{ vPath }, fragPath{fPath}{
        pipelineCount=objPath.size();
        createPoolSize();
        createLayoutBindingSize(tPath.size());
        
        descriptorSet = std::make_shared<LveDescriptorSet>(lveDevice, poolSize, binding);
        descriptorSet->lveTexture = std::make_unique<LveTexture>(lveDevice, tPath);

        createPipeline();


        int i = 0;

        for (auto& loadPtr : lveVecPipeline)
        {
            loadPtr->loadModels(objectPath[i], texPath);
            i++;
        }

        initCommandBuffers();

        lveBufferPtrVec.resize(no_tf_matrices);//Enter count of Transform Matrices

        for (auto& bufferPtr : lveBufferPtrVec)
        {

                bufferPtr = std::make_shared<LveBuffer>(lveDevice, sizeof(transform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                bufferPtr->map(bufferPtr->getBufferSize());
                bufferPtr->writeToBuffer((void*)&transform, sizeof(transform));

        }


        descriptorSet->writeBufferReference(lveBufferPtrVec);
        descriptorSet->writeDescriptorSet();


    }

    //Particle Renderer--------------------------------------------------------------

    LveRenderer::LveRenderer(LveDevice& lDevice,
                             LveWindow& lWindow,
                             LveSwapChain& lSwapChain,
                             std::vector<std::string> vPath,
                             std::vector<std::string> fPath,
                             std::string compPath,
                             std::vector<std::string> tPath,
                             std::vector<Vertex> vertices,
                             std::vector<std::vector<std::string>> objPath)
            : lveDevice{ lDevice }, lveWindow{ lWindow }, lveSwapChain{ lSwapChain }, vertPath{ vPath }, fragPath{fPath}, computePath{compPath}, objectPath{objPath}, vertex{vertices} {

        pipelineCount=objPath.size();



        createPoolSize();
        createLayoutBindingSize(tPath.size());

        descriptorSet = std::make_shared<LveDescriptorSet>(lveDevice, poolSize, binding);
        descriptorSet->lveTexture = std::make_unique<LveTexture>(lveDevice, tPath);

        lveComputePipeline=std::make_unique<LveComputePipeline>(lveDevice,compPath,descriptorSet); //--Still under Testing

        createPipeline();
        indices=lveDevice.findPhysicalQueueFamilies();

        int i = 0;

        for (auto& loadPtr : lveVecPipeline)
        {
            if(i==0)
                loadPtr->loadModelsIndirect(objectPath[i], texPath, 1);
            else
                loadPtr->loadModelsIndirect(objectPath[i], texPath, 1);

            i++;
        }

        initCommandBuffers();




        lveBufferPtrVec.resize(no_tf_matrices);//Enter count of Transform Matrices

        for (auto& bufferPtr : lveBufferPtrVec)
        {

            bufferPtr = std::make_shared<LveBuffer>(lveDevice, sizeof(transform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            bufferPtr->map(bufferPtr->getBufferSize());
            bufferPtr->writeToBuffer((void*)&transform, sizeof(transform));

        }


        descriptorSet->writeBufferReference(lveBufferPtrVec);
        descriptorSet->writeDescriptorSet();

       // createComputeCommandBuffer();
    }

    //-------------------------------------------------------------------------------

    LveRenderer::~LveRenderer() {
        for (auto &pipeline: lveVecPipeline) {
            vkDestroyPipeline(lveDevice.device(), pipeline->graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(lveDevice.device(), pipeline->getPipelineLayout(), nullptr);
            for(auto &delmodel:pipeline->lveModel)
            {
                delmodel->descriptorSet.reset();
            }

            for(auto& pipelineModels: pipeline->lveModel)
                pipelineModels.reset();
        }
        for(auto& delBufPtr: lveBufferPtrVec)
        {
            delBufPtr.reset();
        }

        vkFreeCommandBuffers(lveDevice.device(),lveDevice.getComputeCommandPool(), 1, &computeCommandBuffers);
        vkFreeCommandBuffers(lveDevice.device(),lveDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
        vkDestroyCommandPool(lveDevice.device(),lveDevice.getComputeCommandPool(),nullptr);


        lveComputePipeline->descriptorSet.reset();
        vkDestroyPipeline(lveDevice.device(), lveComputePipeline->computePipeline, nullptr);
        vkDestroyPipelineLayout(lveDevice.device(), lveComputePipeline->computePipelineLayout, nullptr);

        lveVecPipeline.clear();
        lveComputePipeline.reset();
        descriptorSet.reset();
        lveDevice.destroyDevice();
    }



    void LveRenderer::createPoolSize()
    {
        VkDescriptorPoolSize vdps;
        vdps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vdps.descriptorCount = 1;

        poolSize.push_back(vdps);

        vdps.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        vdps.descriptorCount = 1;

        poolSize.push_back(vdps);
        poolSize.push_back(vdps);//normalGL
        poolSize.push_back(vdps);//roughness
        poolSize.push_back(vdps);//emission
        poolSize.push_back(vdps);//jupiter
    }

    void LveRenderer::createLayoutBindingSize(size_t size)
    {
        VkDescriptorSetLayoutBinding MatrixSet;
        MatrixSet.binding = 0;
        MatrixSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        MatrixSet.descriptorCount = 1;
        MatrixSet.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
        MatrixSet.pImmutableSamplers = (VkSampler*)nullptr;

        binding.push_back(MatrixSet);

        MatrixSet.binding = 0;
        MatrixSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        MatrixSet.descriptorCount = 1;
        MatrixSet.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
        MatrixSet.pImmutableSamplers = (VkSampler*)nullptr;

        for (int i = 0; i < 5; i++)
        {
            binding.push_back(MatrixSet);
        }

    }


    void LveRenderer::initCommandBuffers() {
        commandBuffers.clear();
        commandBuffers.resize(lveSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getComputeCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(1);

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, &computeCommandBuffers) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate compute command buffers!");
        }

    }


    void LveRenderer::createPipeline() {

        lveVecPipeline.resize(pipelineCount);

        int pathCtr = 0;

        //assert((pipelineCount == vertPath.size() && pipelineCount == fragPath.size()) && "Error in Path Setup!");

        for (auto& pipeline : lveVecPipeline)
        {
            PipelineConfigInfo pipelineConfig{};
            pipelineConfig.renderPass = lveSwapChain.getRenderPass();

            LvePipeline::defaultPipelineConfigInfo(
                pipelineConfig,
                lveSwapChain.width(),
                lveSwapChain.height()
            );

                //Still Under Testing!---------------------------------------------------------
                /*
                pipeline = std::make_unique<LvePipeline>(
                    lveDevice,
                    vertPath[pathCtr],
                    fragPath[pathCtr],
                    tessCtrlPath[pathCtr],
                    tessEvalPath[pathCtr],
                    pipelineConfig,
                    vertex,
                    texPath,
                    descriptorSet);
                pipelineMap.insert(std::make_pair("Par:"+std::to_string(pathCtr),pathCtr));
                */
                //-----------------------------------------------------------------------------
                if(pathCtr==0) {
                    pipeline = std::make_unique<LvePipeline>(
                            lveDevice,
                            vertPath[pathCtr],
                            fragPath[pathCtr],
                            pipelineConfig,
                            texPath,
                            objectPath[pathCtr],
                            descriptorSet,
                            true,
                            false);
                    pipelineMap.insert(std::make_pair("P:" + std::to_string(pathCtr), pathCtr));
                }
                else{
                    pipeline = std::make_unique<LvePipeline>(
                            lveDevice,
                            vertPath[pathCtr],
                            fragPath[pathCtr],
                            pipelineConfig,
                            texPath,
                            objectPath[pathCtr],
                            descriptorSet,
                            true,
                            false);
                    pipelineMap.insert(std::make_pair("P:" + std::to_string(pathCtr), pathCtr));
                }



            pathCtr++;
        }

    }

    void LveRenderer::createComputeCommandBuffer() {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(computeCommandBuffers, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        if (indices.graphicsFamily != indices.computeFamily)
        {
            VkBufferMemoryBarrier buffer_barrier =
                    {
                            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                            nullptr,
                            0,
                            VK_ACCESS_SHADER_WRITE_BIT,
                            indices.graphicsFamily,
                            indices.computeFamily,
                            lveComputePipeline->ParticleBuffer,
                            0,
                            lveComputePipeline->particleBufferSize
                    };

            vkCmdPipelineBarrier(
                    computeCommandBuffers,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0,
                    0, nullptr,
                    1, &buffer_barrier,
                    0, nullptr);
        }

        vkCmdBindPipeline(computeCommandBuffers, VK_PIPELINE_BIND_POINT_COMPUTE, lveComputePipeline->computePipeline);
        vkCmdBindDescriptorSets(computeCommandBuffers, VK_PIPELINE_BIND_POINT_COMPUTE, lveComputePipeline->computePipelineLayout, 0, 2, lveComputePipeline->descriptorSet->computeDescriptor, 0, 0);
        vkCmdDispatch(computeCommandBuffers, 2, 1, 1);

        if (indices.graphicsFamily != indices.computeFamily)
        {
            VkBufferMemoryBarrier buffer_barrier =
                    {
                            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                            nullptr,
                            VK_ACCESS_SHADER_WRITE_BIT,
                            0,
                            indices.computeFamily,
                            indices.graphicsFamily,
                            lveComputePipeline->ParticleBuffer,
                            0,
                            lveComputePipeline->particleBufferSize
                    };

            vkCmdPipelineBarrier(
                    computeCommandBuffers,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0,
                    0, nullptr,
                    1, &buffer_barrier,
                    0, nullptr);
        }

        vkEndCommandBuffer(computeCommandBuffers);
    }

    void LveRenderer::createCommandBuffers(std::vector<std::string> pipelineNames) {
        for (int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = lveSwapChain.getRenderPass();
            renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            int j = 0;
            std::unordered_map<std::string, uint32_t>::iterator pipCheck;

            for (auto &pipeline: lveVecPipeline) {
                pipeline->bind(commandBuffers[i]);
                pipeline->DescriptorbindPipeline(commandBuffers[i], descriptorSet->descriptor);
                for (auto &models: pipeline->lveModel) {
                    if(!pipeline->indirectDraw) {
                        models->bind(commandBuffers[i]);
                        models->indexbind(commandBuffers[i]);
                        models->indexdraw(commandBuffers[i]);
                    }
                    else {
                        VkDrawIndirectCommand indirectCmd;
                        VkDeviceSize offsets[] = {0};
                        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &models->vertexBuffer, offsets);
                        vkCmdBindVertexBuffers(commandBuffers[i], 1, 1, &lveComputePipeline->ParticleBuffer, offsets);
                        vkCmdDrawIndirect(commandBuffers[i],models->indDrawCmdBuffer,0,1,sizeof (indirectCmd));
                    }
                }
            j++;
            }


            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
    void LveRenderer::drawFrame() {
        uint32_t imageIndex;

        auto result = lveSwapChain.acquireNextImage(&imageIndex);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        lveSwapChain.computeSubmitCommandBuffer(computeCommandBuffers, &imageIndex);

        result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}