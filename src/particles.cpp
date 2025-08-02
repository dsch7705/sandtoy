#include "particles.h"

#include <stdexcept>


ParticlePhase ParticleState::phase() const
{
    if (!kParticleProperties.contains(type))
    {
        throw std::runtime_error("No entry for ParticleType '" + kParticleTypeNames[static_cast<int>(type)] + "' " + "in kParticleProperties");
    }
    
    return kParticleProperties.at(type).phase;
}