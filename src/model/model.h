# pragma once

#include "texture.h"
#include "mesh.h"

#include <vulkan/vulkan.h>

#include <vector>

class Resources;

class Model
{
public:
    Model();
    void cmdBindBuffers(VkCommandBuffer commandBuffer) const;
    void cmdDrawIndexed(VkCommandBuffer commandBuffer) const;
    void cleanUp(VkDevice device);
    void loadModel(const char* filename);
    VkDescriptorImageInfo getTextureDescriptorImageInfo() const;
private:
    // Images
    Texture m_texture;

    Resources* m_appResources;
    VkBuffer m_vertexBuffer;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkDeviceMemory m_indexBufferMemory;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};