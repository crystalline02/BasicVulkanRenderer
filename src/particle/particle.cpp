#include "particle.h"

#include "../resources.h"

#include <random>
#include <time.h>


void ParticleGroup::initParticleGroup(uint32_t particleCount)
{
    m_resources = Resources::get();

    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> dist(0.1f, 1.f);

    // Fill in particle data
    m_particles.resize(particleCount);
    for(uint32_t i = 0; i < particleCount; ++i)
    {
        float r = 0.5f * glm::sqrt(dist(rndEngine));
        float theta = 2.f * 3.1415926535 * dist(rndEngine);
        float x = r * glm::cos(theta) * (m_resources->windowSize().height / m_resources->windowSize().width);
        float y = r * glm::sin(theta);
        m_particles[i].position = {x, y};
        m_particles[i].velosity = glm::normalize(m_particles[i].position) * 0.00025f;
        m_particles[i].color = {dist(rndEngine), dist(rndEngine), dist(rndEngine), 1.f};
    }

    m_resources->createParticleUBOs(m_particleUBOs, m_particleUBOMemories, m_particleUBOMapped);
    m_resources->createParticleSSBOs(m_particleSSBOs, m_particleSSBOMemories, m_particles);
}

void ParticleGroup::recordUpdateCommandBuffer(VkCommandBuffer commandBuffer)
{
    m_resources->recordUpdateParticleCommandBuffer(commandBuffer, static_cast<uint32_t>(m_particles.size()));
}

ParticleGroup::ParticleGroup(uint32_t particleCount)
{
    initParticleGroup(particleCount);
}

void ParticleGroup::allocateDescriptorSet(VkDescriptorPool descriptorPool)
{
    m_resources->allocateParticleDescriptorSets(m_particleDescriptorSets, m_particleUBOs, m_particleSSBOs);
}

void ParticleGroup::updateParticle()
{

}

void ParticleGroup::updateUniformBuffers(uint32_t frameIndex, UBOParticle uboParticle)
{
    memcpy(m_particleUBOs[frameIndex], &uboParticle, sizeof(UBOParticle));
}

void ParticleGroup::allocateDrawCommandBuffers()
{
    m_resources->allocateDrawParticleCommanBuffers(m_drawCommandBuffers);
}

void ParticleGroup::cleanUp(VkDevice device, uint32_t maxInFlightFence)
{
    for(uint32_t i = 0; i < maxInFlightFence; ++i)
    {
        vkDestroyBuffer(device, m_particleUBOs[i], VK_NULL_HANDLE);
        vkFreeMemory(device, m_particleUBOMemories[i], VK_NULL_HANDLE);

        vkDestroyBuffer(device, m_particleSSBOs[i], VK_NULL_HANDLE);
        vkFreeMemory(device, m_particleSSBOMemories[i], VK_NULL_HANDLE);
    }
}