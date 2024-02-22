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
    ParticleGroup(uint32_t size);

private:
    std::vector<Particle> m_particles;
    std::vector<VkBuffer> m_particleSSBOs;
    std::vector<VkDeviceMemory> m_particleSSBOMemories;
    std::vector<VkDescriptorSet> m_particleDescriptorSets; 
    Resources* m_resources;
    
};