# pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class Resources;

struct Particle
{
    glm::vec2 position;
    glm::vec2 velosity;
    glm::vec4 color;
};

class ParticleGroup
{
public:
    struct UBOParticle
    {
        float deltaTime;
        uint32_t particleCount;
    };
    ParticleGroup(uint32_t particleCount);
    void recordUpdateCommandBuffer(VkCommandBuffer commandBuffer);
    void allocateDescriptorSet(VkDescriptorPool descriptorPool);
    void allocateDrawCommandBuffers();
    void updateUniformBuffers(uint32_t frameIndex, UBOParticle uboParticle);
    uint32_t particleBufferSize() const { return m_particles.size() * sizeof(Particle); }
    void updateParticle();
    void cleanUp(VkDevice device, uint32_t maxInFlightFence);
private:
    std::vector<Particle> m_particles;
    std::vector<VkBuffer> m_particleUBOs;
    std::vector<VkDeviceMemory> m_particleUBOMemories;
    std::vector<VkBuffer> m_particleSSBOs;
    std::vector<VkDeviceMemory> m_particleSSBOMemories;
    std::vector<void*> m_particleUBOMapped;
    std::vector<VkDescriptorSet> m_particleDescriptorSets; 
    std::vector<VkCommandBuffer> m_drawCommandBuffers;
    VkPipeline m_graphicPipeline,
        m_computePipeline;
    VkPipelineLayout m_graphicPipelineLayout;

    Resources* m_resources;

    void initParticleGroup(uint32_t particleCount);
};