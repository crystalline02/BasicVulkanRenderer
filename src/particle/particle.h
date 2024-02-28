# pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class Resources;

struct Particle
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 velosity;
};

class ParticleGroup
{
public:
    struct UBOParticle
    {
        float deltaTime;
        uint32_t particleCount;
    };
    ParticleGroup();
    void allocateDescriptorSet();
    void createDescriptorSetLayout();
    void updateUniformBuffers(uint32_t frameIndex, float deltaTime);
    void createComputePipeline();
    void createGraphicPipeline();
    void cmdDrawParticles(VkCommandBuffer commandBuffer, uint32_t frameIndex);
    void cmdUpdateParticles(VkCommandBuffer commandBuffer, uint32_t frameIndex);
    void cleanUp(VkDevice device, uint32_t maxInFlightFence);
    void initParticleGroup(uint32_t particleCount);

    uint32_t particleBufferSize() const { return m_particles.size() * sizeof(Particle); }
    uint32_t uniformDescriptorCount() const { return 2; }
    uint32_t combinedImageSamplerCount() const { return 0; }
    uint32_t storageDescriptorCount() const { return 2; }
private:
    std::vector<Particle> m_particles;
    std::vector<VkBuffer> m_particleUBOs;
    std::vector<VkDeviceMemory> m_particleUBOMemories;
    std::vector<VkBuffer> m_particleSSBOs;
    std::vector<VkDeviceMemory> m_particleSSBOMemories;
    std::vector<void*> m_particleUBOMapped;
    std::vector<VkDescriptorSet> m_computeDescriptorSets,
        m_graphicDescriptorSets;
    VkBuffer particleVertexBuffer;
    VkDeviceMemory particleVertexBufferMemory;
    VkPipeline m_graphicPipeline,
        m_computePipeline;
    VkPipelineLayout m_graphicPipelineLayout,
        m_computePipelineLayout;
    VkDescriptorSetLayout m_computeDescriptorSetLayout,
        m_graphicDescriptorSetLayout;

    Resources* m_resources;

};