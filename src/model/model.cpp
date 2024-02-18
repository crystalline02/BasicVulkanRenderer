#include "model.h"

#include <vulkan/vulkan.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>

#include "mesh.h"
#include "../resources.h"

Model::Model(const char* filename, Resources* appResources): m_appResources(appResources)
{
    loadModel(filename);
}

void Model::cmdBindBuffers(VkCommandBuffer commandBuffer) const
{
    VkDeviceSize offsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, &offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::cmdDrawIndexed(VkCommandBuffer commandBuffer) const
{
    vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
}

void Model::cleanUp(VkDevice device)
{
    vkDestroyBuffer(device, m_indexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(device, m_indexBufferMemory, VK_NULL_HANDLE);
    vkDestroyBuffer(device, m_vertexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(device, m_vertexBufferMemory, VK_NULL_HANDLE);

    vkDestroySampler(device, m_texture.sampler, VK_NULL_HANDLE);
    vkDestroyImageView(device, m_texture.imageView, VK_NULL_HANDLE);
    vkDestroyImage(device, m_texture.image, VK_NULL_HANDLE);
    vkFreeMemory(device, m_texture.imageMemory, VK_NULL_HANDLE);
}

void Model::loadModel(const char* filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename))
        throw std::runtime_error("TINYOBJLOADER ERROR: Failed to load model from " + std::string(filename) + ".");

    std::unordered_map<Vertex, uint32_t> uniqueVertex;
    for(const tinyobj::shape_t& shape: shapes)
    {
        for(const tinyobj::index_t tinyobjVertex : shape.mesh.indices)
        {
            Vertex vertex{
                .position = {attrib.vertices[3 * tinyobjVertex.vertex_index + 0], attrib.vertices[3 * tinyobjVertex.vertex_index + 1], attrib.vertices[3 * tinyobjVertex.vertex_index + 2]},
                .normal = {attrib.normals[3 * tinyobjVertex.normal_index + 0], attrib.normals[3 * tinyobjVertex.normal_index + 1], attrib.normals[3 * tinyobjVertex.normal_index + 2]},
                .texCoord = {attrib.texcoords[2 * tinyobjVertex.texcoord_index + 0], 1.f - attrib.texcoords[2 * tinyobjVertex.texcoord_index + 1]}
            };

            if(uniqueVertex.find(vertex) == uniqueVertex.end())
            {
                uniqueVertex[vertex] = static_cast<uint32_t>(m_vertices.size());
                m_vertices.emplace_back(vertex);
            }
            m_indices.emplace_back(uniqueVertex[vertex]);
        }
    }

    m_appResources->createVertexBuffer(m_vertices, m_vertexBuffer, m_vertexBufferMemory);
    m_appResources->createIndexBuffer(m_indices, m_indexBuffer, m_indexBufferMemory);

    m_appResources->createTexture("./textures/viking_room.png", m_texture);
}

VkDescriptorImageInfo Model::getTextureDescriptorImageInfo() const
{
    VkDescriptorImageInfo descriptorImageInfo = {
        .sampler = m_texture.sampler,
        .imageView = m_texture.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    return descriptorImageInfo;
}