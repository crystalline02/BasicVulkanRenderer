# pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>

#include <vector>


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const
    {
        return (position == other.position) && (normal == other.normal) && (texCoord == other.texCoord);
    }
};

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& vertex) const
        {
            return (hash<glm::vec3>()(vertex.position)) ^ 
                ((hash<glm::vec3>()(vertex.normal) << 1) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
};

class Resources;

class Model
{
public:
    Model(const char* filename, Resources* appResources);

    void cmdBindBuffers(VkCommandBuffer commandBuffer) const;
    void cmdDrawIndexed(VkCommandBuffer commandBuffer) const;
    void cleanUp(VkDevice device);
    VkDescriptorImageInfo getTextureDescriptorImageInfo() const;

private:
    void loadModel(const char* filename);

    // Images
    VkImage m_textureImage;
    VkImageView m_textureImageView;
    VkDeviceMemory m_textureImageMemory;
    VkSampler m_textureSampler;

    Resources* m_appResources;
    VkBuffer m_vertexBuffer;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkDeviceMemory m_indexBufferMemory;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};