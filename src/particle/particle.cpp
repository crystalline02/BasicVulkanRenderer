#include "particle.h"

#include "../resources.h"

#include <random>
#include <time.h>

ParticleGroup::ParticleGroup(uint32_t size)
{
    m_resources = Resources::get();

    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> dist(0.1f, 1.f);

    // Fill in particle data
    m_particles.resize(size);
    for(uint32_t i = 0; i < size; ++i)
    {
        float r = 0.5f * glm::sqrt(dist(rndEngine));
        float theta = 2.f * 3.1415926535 * dist(rndEngine);
        float x = r * glm::cos(theta) * (m_resources->windowSize().height / m_resources->windowSize().width);
        float y = r * glm::sin(theta);
        m_particles[i].position = {x, y};
        m_particles[i].velosity = glm::normalize(m_particles[i].position) * 0.00025f;
        m_particles[i].color = {dist(rndEngine), dist(rndEngine), dist(rndEngine), 1.f};
    }

    m_resources->createParticleSSBOs(m_particleSSBOs, m_particleSSBOMemories, m_particles);
}