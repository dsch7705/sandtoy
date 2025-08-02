#pragma once


enum class ParticlePhase
{
    Solid,
    Liquid,
    Gas,
    Static
};
#define PARTICLE_LIST \
    X(Sand, ParticlePhase::Solid) \
    X(Stone, ParticlePhase::Static) \
    X(Water, ParticlePhase::Liquid) \
    X(Gravel, ParticlePhase::Solid) \
    X(Dirt, ParticlePhase::Solid) \
    X(Blue, ParticlePhase::Solid) \
    X(Pink, ParticlePhase::Solid) \
    X(Rainbow, ParticlePhase::Solid) \
    X(Air, ParticlePhase::Static) \
    X(Steam, ParticlePhase::Gas)

enum class ParticleType
{
#define X(NAME, PHASE) NAME,
    PARTICLE_LIST
#undef X
    COUNT
};
static constexpr const char* ParticleTypeNames[] = 
{
#define X(NAME, PHASE) #NAME,
    PARTICLE_LIST
#undef X
};
static constexpr ParticlePhase ParticlePhases[] =
{
#define X(NAME, PHASE) PHASE,
    PARTICLE_LIST
#undef X
};

struct ParticleState
{
    ParticleType type;
    float temperature;

    ParticlePhase phase() const;

    bool operator==(const ParticleState& other) const
    {
        return (type == other.type && temperature == other.temperature);
    }
};
constexpr ParticleState defaultParticleState(ParticleType type)
{
    switch (type)
    {
    case ParticleType::Steam:
        return { .type = ParticleType::Steam, .temperature = 100 };

    default:
        return { .type = type, .temperature = 0 };

    }
}