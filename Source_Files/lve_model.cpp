#include "lve_model.hpp"

// std
#include <cassert>
#include <cstring>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace lve {

    LveModel::LveModel(LveDevice& device, const std::vector<Vertex>& vertices, std::vector<std::string> filePath, std::shared_ptr<LveDescriptorSet> descSet) : lveDevice{ device }, descriptorSet{descSet} {
  createVertexBuffers(vertices);
}

LveModel::LveModel(LveDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& index, std::vector<std::string> filePath, std::shared_ptr<LveDescriptorSet> descSet) : lveDevice{ device }, descriptorSet{ descSet }
{
  createVertexBuffers(vertices);
  createIndexBuffers(index);
}

LveModel::LveModel(LveDevice& device, std::string objPath, std::vector<std::string> filePath, std::shared_ptr<LveDescriptorSet> descSet, bool drawIndirect) : lveDevice{ device }, descriptorSet{ descSet }, indirect{drawIndirect} {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t>shapes;
    std::vector<tinyobj::material_t>materials;
    std::string error;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, &error, objPath.c_str()))
        throw std::runtime_error(error);

    uint32_t indexCtr=0;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                    1.0

                };

                auto colorIndex = 3 * index.vertex_index + 2;
                if (colorIndex < attrib.colors.size()) {
                    vertex.color = {
                        attrib.colors[colorIndex - 2],
                        attrib.colors[colorIndex - 1],
                        attrib.colors[colorIndex - 0],
                        1.0
                    };
                }
                else {
                    vertex.color = { 0.f, 1.f, 1.f, 1.f };  // set default color
                }
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                    1.0
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                    1.0,
                    1.0
                };
            }
            vertex.color = { 0.f, 1.f, 1.f, 1.f };


            vertices.push_back(vertex);
            indices.push_back(static_cast<uint32_t>(indexCtr));
            indexCtr++;
        }

    }


    if(!indirect) {
        createVertexBuffers(vertices);
        createIndexBuffers(indices);
    }
}

LveModel::~LveModel() {
     if(!indirect) {
        vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
        vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
        vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
     }
     else
     {
         vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
         vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
         vkFreeMemory(lveDevice.device(), indDrawCmdMemory, nullptr);
         vkDestroyBuffer(lveDevice.device(), indDrawCmdBuffer, nullptr);
     }
}

void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  lveDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      vertexBuffer,
      vertexBufferMemory);

  void *data;
  vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(lveDevice.device(), vertexBufferMemory);
}

void LveModel::updateIndirectBuffer(int drawCount) {
    VkDrawIndirectCommand indirectCmd{};
    indirectCmd.instanceCount = drawCount;
    indirectCmd.firstInstance = 0;
    indirectCmd.firstVertex = 0;
    indirectCmd.vertexCount = vertices.size();
    // @todo: Multiple primitives
    // A glTF node may consist of multiple primitives, so we may have to do multiple commands per mesh

    indirectCommands.push_back(indirectCmd);

    VkDeviceSize bufferSize = sizeof(indirectCommands[0])*indirectCommands.size();

    lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indDrawCmdBuffer,
            indDrawCmdMemory);

    void *data;
    vkMapMemory(lveDevice.device(),indDrawCmdMemory, 0, bufferSize, 0, &data);
    memcpy(data, indirectCommands.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(lveDevice.device(), indDrawCmdMemory);
}

void LveModel::createVertexBuffersIndirect(const std::vector<Vertex> &vertices, int drawCount){

    updateIndirectBuffer(drawCount);

    vertexCount = static_cast<uint32_t>(vertices.size());

    int bufferSize = sizeof(vertices[0]) * vertexCount;

    lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingVertexBuffer,
            stagingVertexBufferMemory);

    lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBuffer,
            vertexBufferMemory);

    void *data;

    vkMapMemory(lveDevice.device(), stagingVertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(lveDevice.device(), stagingVertexBufferMemory);

    lveDevice.copyBuffer(stagingVertexBuffer,vertexBuffer,bufferSize);

    vkFreeMemory(lveDevice.device(), stagingVertexBufferMemory, nullptr);
    vkDestroyBuffer(lveDevice.device(), stagingVertexBuffer, nullptr);
}

void LveModel::createIndexBuffers(const std::vector<uint32_t>& index) {
    indexCount = static_cast<uint32_t>(index.size());
    assert(indexCount >= 3 && "Index count must be at least 3");
    VkDeviceSize bufferSize = sizeof(index[0]) * indexCount;
    lveDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBuffer,
        indexBufferMemory);

    void* data;
    vkMapMemory(lveDevice.device(), indexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, index.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(lveDevice.device(), indexBufferMemory);
}

void LveModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

void LveModel::indexdraw(VkCommandBuffer commandBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}



void LveModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void LveModel::indexbind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers = { indexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindIndexBuffer(commandBuffer, buffers, 0, VK_INDEX_TYPE_UINT32);
}
}  // namespace lve