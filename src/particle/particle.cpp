#include "particle.h"

#include "../resources.h"

#include <random>
#include <time.h>
#include <iostream>

void ParticleGroup::initParticleGroup(uint32_t particleCount)
{
    m_resources = Resources::get();

    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> dist(0.1f, 1.f);

    // Fill in particle data
    m_particles.resize(particleCount);
    for(uint32_t i = 0; i < particleCount; ++i)
    {
        float posX = (dist(rndEngine) - 0.5f) * 1.6f,
            posY = (dist(rndEngine) - 0.5f) * 1.6f,
            posZ = dist(rndEngine) * 2.2f + 1.f;

        float velR = 3.f * dist(rndEngine);
        float velTheta = 2.f * 3.1415926535f * dist(rndEngine);
        float velPhy = 3.1415926535f * dist(rndEngine);
        float velX = velR * glm::sin(velPhy) * glm::cos(velTheta),
            velY = velR * glm::sin(velPhy) * glm::sin(velTheta),
            velZ = velR * glm::cos(velPhy);
        m_particles[i].position = {posX, posY, posZ};
        m_particles[i].velosity = {velX, velY, velZ};
        m_particles[i].color = {dist(rndEngine), dist(rndEngine), dist(rndEngine), 1.f};
    }
    
    m_resources->createParticleUBOs(m_particleUBOs, m_particleUBOMemories, m_particleUBOMapped);
    m_resources->createParticleSSBOs(m_particleSSBOs, m_particleSSBOMemories, m_particles);
}

void ParticleGroup::cmdUpdateParticles(VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
    m_resources->cmdUpdateParticles(commandBuffer, 
        m_computePipeline, 
        m_computePipelineLayout, 
        m_computeDescriptorSets[frameIndex], 
        static_cast<uint32_t>(m_particles.size()));
}

void ParticleGroup::cmdDrawParticles(VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
    m_resources->cmdDrawParticles(commandBuffer, 
        m_graphicPipeline,
        m_graphicPipelineLayout,
        m_particleSSBOs[frameIndex], 
        m_graphicDescriptorSets[frameIndex],
        static_cast<uint32_t>(m_particles.size()));
}

ParticleGroup::ParticleGroup()
{
    m_resources = Resources::get();
}

void ParticleGroup::allocateDescriptorSet()
{
    m_resources->allocateParticleDescriptorSets(m_computeDescriptorSets,
        m_graphicDescriptorSets, 
        m_particleUBOs, 
        m_particleSSBOs, 
        m_graphicDescriptorSetLayout,
        m_computeDescriptorSetLayout);
}

void ParticleGroup::updateUniformBuffers(uint32_t frameIndex, float deltaTime)
{
    UBOParticle uboParticle = {
        .deltaTime = deltaTime,
        .particleCount = static_cast<uint32_t>(m_particles.size())
    };
    memcpy(m_particleUBOMapped[frameIndex], &uboParticle, sizeof(UBOParticle));
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
    vkDestroyDescriptorSetLayout(device, m_computeDescriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(device, m_graphicDescriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(device, m_graphicPipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(device, m_computePipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipeline(device, m_graphicPipeline, VK_NULL_HANDLE);
    vkDestroyPipeline(device, m_computePipeline, VK_NULL_HANDLE);
}

void ParticleGroup::createGraphicPipeline()
{
    m_resources->createParticleGraphicPipeline(m_graphicPipeline, m_graphicPipelineLayout, m_graphicDescriptorSetLayout);
}

void ParticleGroup::createDescriptorSetLayout()
{
    m_resources->createParticlesDescriptorSetLayout(m_computeDescriptorSetLayout, m_graphicDescriptorSetLayout);
}

void ParticleGroup::createComputePipeline()
{
    m_resources->createParticleComputePipeline(m_computePipeline, m_computePipelineLayout, m_computeDescriptorSetLayout);
}