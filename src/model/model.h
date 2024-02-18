# pragma once

#include "texture.h"
#include "mesh.h"

#include <vulkan/vulkan.h>

#include <vector>

class Resources;

class Model
{
public:
    Model(const char* filename);

    void cmdBindBuffers(VkCommandBuffer commandBuffer) const;
    void cmdDrawIndexed(VkCommandBuffer commandBuffer) const;
    void cleanUp(VkDevice device);
    VkDescriptorImageInfo getTextureDescriptorImageInfo() const;

private:
    void loadModel(const char* filename);

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