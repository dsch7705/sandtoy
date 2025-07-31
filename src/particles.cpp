#include "particles.h"


ParticlePhase ParticleState::phase() const
{
    return ParticlePhases[static_cast<int>(type)];
}