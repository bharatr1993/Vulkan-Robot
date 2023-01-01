#include "lve_descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace lve {

// *************** Descriptor Set Layout *********************

LveDescriptorSet::LveDescriptorSet(
    LveDevice &lveDevice, std::vector<VkDescriptorPoolSize> pSizes, std::vector<VkDescriptorSetLayoutBinding> bindings)
    : lveDevice{ lveDevice }, poolSizes{ pSizes }, binding{bindings} {
  createPool();
  
  int i = 0;

  descriptorSetLayout.resize(binding.size());
  computeDescriptorSetLayout.resize(2);

  for (auto& bind : binding)
  {
      VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
      descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(1);
      descriptorSetLayoutInfo.pBindings = &bind;

      if (vkCreateDescriptorSetLayout(
          lveDevice.device(),
          &descriptorSetLayoutInfo,
          nullptr,
          &descriptorSetLayout[i]) != VK_SUCCESS) {
          throw std::runtime_error("failed to create descriptor set layout!");
      }
      i++;
  }


    computeBinding[0].binding = 0;
    computeBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    computeBinding[0].descriptorCount = 1;
    computeBinding[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    computeBinding[0].pImmutableSamplers = (VkSampler*)nullptr;

    computeBinding[1].binding = 0;
    computeBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    computeBinding[1].descriptorCount = 1;
    computeBinding[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    computeBinding[1].pImmutableSamplers = (VkSampler*)nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo[2]={};

    descriptorSetLayoutInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo[0].bindingCount = static_cast<uint32_t>(1);
    descriptorSetLayoutInfo[0].pBindings = &computeBinding[0];

    descriptorSetLayoutInfo[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo[1].bindingCount = static_cast<uint32_t>(1);
    descriptorSetLayoutInfo[1].pBindings = &computeBinding[1];

    for(i=0;i<2;i++) {
        if (vkCreateDescriptorSetLayout(
                lveDevice.device(),
                &descriptorSetLayoutInfo[i],
                nullptr,
                &computeDescriptorSetLayout[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute descriptor set layout: Misc!");
        }
    }


  allocateDescriptor();
}

LveDescriptorSet::~LveDescriptorSet() {
    for(auto descLayout: descriptorSetLayout)
        vkDestroyDescriptorSetLayout(lveDevice.device(), descLayout, nullptr);
  vkDestroyDescriptorPool(lveDevice.device(), descPool, nullptr);
  for(auto& buf:buffers)
      buf.reset();
  lveTexture.reset();
    vkDestroyDescriptorSetLayout(lveDevice.device(), computeDescriptorSetLayout[0], nullptr);
    vkDestroyDescriptorSetLayout(lveDevice.device(), computeDescriptorSetLayout[1], nullptr);
    vkDestroyDescriptorPool(lveDevice.device(), computeDescPool, nullptr);


}

// *************** Descriptor Pool *********************

void LveDescriptorSet::createPool()
  {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(lveDevice.device(), &descriptorPoolInfo, nullptr, &descPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }

  VkDescriptorPoolSize vdps[2];
  vdps[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  vdps[0].descriptorCount = 1;
  vdps[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  vdps[1].descriptorCount = 1;

  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(2);
  descriptorPoolInfo.pPoolSizes = &vdps[0];
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(lveDevice.device(), &descriptorPoolInfo, nullptr, &computeDescPool) !=
      VK_SUCCESS) {
      throw std::runtime_error("failed to create compute descriptor pool!");
  }
}
/*
void LveDescriptorSet::initParticleBuffer() {
    particlebuf pushparticle={};
    for (int i=0;i<10;i++) {
        float x = ((rand()%200)/100.0f)-1.0;
        float y = ((rand()%200)/100.0f)-1.0;
        pushparticle.position=glm::vec2(x,y);
        pushparticle.velocity=glm::vec2(0.1f);
        //pushparticle.totalcount=100;
        particlebuffer.push_back(pushparticle);

    }
}


void LveDescriptorSet::prepareParticleBuffer() {
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

    lveDevice.copyBuffer(stagingParticleBuffer,ParticleBuffer,particleBufferSize);

    vkFreeMemory(lveDevice.device(), stagingParticleBufferMemory, nullptr);
    vkDestroyBuffer(lveDevice.device(), stagingParticleBuffer, nullptr);
}
*/

void LveDescriptorSet::updateTransformDescriptorSet() {
    VkDescriptorBufferInfo vdbi0;

    VkWriteDescriptorSet vwds0;

    vdbi0.range = sizeof(Transform);
    vdbi0.buffer = buffers[0]->getBuffer();//Change for vec impl
    vdbi0.offset = 0;	// bytes
    vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vwds0.pNext = nullptr;
    vwds0.dstSet = descriptor[0];
    vwds0.dstBinding = 0;
    vwds0.dstArrayElement = 0;
    vwds0.descriptorCount = 1;
    vwds0.pTexelBufferView = (VkBufferView*)nullptr;
    vwds0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vwds0.pBufferInfo = &vdbi0;
    vwds0.pImageInfo = (VkDescriptorImageInfo*)nullptr;

    vkUpdateDescriptorSets(lveDevice.device(), 1, &vwds0, 0, (VkCopyDescriptorSet*)nullptr);

}

void LveDescriptorSet::writeDescriptorSet() {
    // ds 0:
    int i = 0;
   // assert((descriptor.size() == buffers.size()) && "Error in size: mismatch");
    VkDescriptorBufferInfo vdbi0;
    VkDescriptorImageInfo vdii0;
   // descriptor.resize(1);

    for (auto& writeDescSet : descriptor)
    {
        VkWriteDescriptorSet vwds0;

        switch (i)
        {
        case 0: 
            vdbi0.range = sizeof(Transform);
            vdbi0.buffer = buffers[i]->getBuffer();//Change for vec impl
            vdbi0.offset = 0;	// bytes
            vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vwds0.pNext = nullptr;
            vwds0.dstSet = writeDescSet;
            vwds0.dstBinding = 0;
            vwds0.dstArrayElement = 0;
            vwds0.descriptorCount = 1;
            vwds0.pTexelBufferView = (VkBufferView*)nullptr;
            vwds0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            vwds0.pBufferInfo = &vdbi0;
            vwds0.pImageInfo = (VkDescriptorImageInfo*)nullptr;
            break;
        case 1:
            vdii0.sampler = lveTexture->texFile[0].texSampler;
            vdii0.imageView = lveTexture->texFile[0].texImageView;
            vdii0.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vwds0.pNext = nullptr;
            vwds0.dstSet = writeDescSet;
            vwds0.dstBinding = 0;
            vwds0.dstArrayElement = 0;
            vwds0.descriptorCount = 1;
            vwds0.pTexelBufferView = (VkBufferView*)nullptr;
            vwds0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            vwds0.pBufferInfo = (VkDescriptorBufferInfo*)nullptr;
            vwds0.pImageInfo = &vdii0;
            break;
        case 2:
            vdii0.sampler = lveTexture->texFile[1].texSampler;
            vdii0.imageView = lveTexture->texFile[1].texImageView;
            vdii0.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vwds0.pNext = nullptr;
            vwds0.dstSet = writeDescSet;
            vwds0.dstBinding = 0;
            vwds0.dstArrayElement = 0;
            vwds0.descriptorCount = 1;
            vwds0.pTexelBufferView = (VkBufferView*)nullptr;
            vwds0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            vwds0.pBufferInfo = (VkDescriptorBufferInfo*)nullptr;
            vwds0.pImageInfo = &vdii0;
            break;
        case 3:
            vdii0.sampler = lveTexture->texFile[2].texSampler;
            vdii0.imageView = lveTexture->texFile[2].texImageView;
            vdii0.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vwds0.pNext = nullptr;
            vwds0.dstSet = writeDescSet;
            vwds0.dstBinding = 0;
            vwds0.dstArrayElement = 0;
            vwds0.descriptorCount = 1;
            vwds0.pTexelBufferView = (VkBufferView*)nullptr;
            vwds0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            vwds0.pBufferInfo = (VkDescriptorBufferInfo*)nullptr;
            vwds0.pImageInfo = &vdii0;
            break;
        case 4:
            vdii0.sampler = lveTexture->texFile[3].texSampler;
            vdii0.imageView = lveTexture->texFile[3].texImageView;
            vdii0.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vwds0.pNext = nullptr;
            vwds0.dstSet = writeDescSet;
            vwds0.dstBinding = 0;
            vwds0.dstArrayElement = 0;
            vwds0.descriptorCount = 1;
            vwds0.pTexelBufferView = (VkBufferView*)nullptr;
            vwds0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            vwds0.pBufferInfo = (VkDescriptorBufferInfo*)nullptr;
            vwds0.pImageInfo = &vdii0;
            break;
        case 5:
            vdii0.sampler = lveTexture->texFile[4].texSampler;
            vdii0.imageView = lveTexture->texFile[4].texImageView;
            vdii0.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vwds0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vwds0.pNext = nullptr;
            vwds0.dstSet = writeDescSet;
            vwds0.dstBinding = 0;
            vwds0.dstArrayElement = 0;
            vwds0.descriptorCount = 1;
            vwds0.pTexelBufferView = (VkBufferView*)nullptr;
            vwds0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            vwds0.pBufferInfo = (VkDescriptorBufferInfo*)nullptr;
            vwds0.pImageInfo = &vdii0;
            break;
        default:
            assert("Error in Matrix Setup");
        }
        
        i++;
        vkUpdateDescriptorSets(lveDevice.device(), 1, &vwds0, 0, (VkCopyDescriptorSet*)nullptr);
    }

}

bool LveDescriptorSet::allocateDescriptor() {
  VkDescriptorSetAllocateInfo allocInfo{};
  int i = 0;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  descriptor.resize(binding.size());

  for (auto& desc : descriptor)
  {
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descPool;
      allocInfo.pSetLayouts = &descriptorSetLayout[i];
      allocInfo.descriptorSetCount = 1;
      if (vkAllocateDescriptorSets(lveDevice.device(), &allocInfo, &desc) != VK_SUCCESS) {
          return false;
      }
      i++;
  }

    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = computeDescPool;
    allocInfo.pSetLayouts = &computeDescriptorSetLayout[0];
    allocInfo.descriptorSetCount = 2;
    if (vkAllocateDescriptorSets(lveDevice.device(), &allocInfo, &computeDescriptor[0]) != VK_SUCCESS) {
        return false;
    }
      return true;
  
}

void LveDescriptorSet::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(
      lveDevice.device(),
      descPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}

void LveDescriptorSet::resetPool() {
    vkResetDescriptorPool(lveDevice.device(), descPool, 0);
}

// *************** Descriptor Writer *********************

void LveDescriptorSet::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(lveDevice.device(), writes.size(), writes.data(), 0, nullptr);
}

}  // namespace lve

