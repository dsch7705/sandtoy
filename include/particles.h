#pragma once


#define PARTICLE_TYPE_LIST \
    X(Air) \
    X(Sand) \
    X(Gravel) \
    X(Dirt) \
    X(Stone) \
    X(Rainbow) \
    X(Pink) \
    X(Blue) \
    X(Water) \
    X(Steam)

enum class ParticleType
{
#define X(NAME) NAME,
    PARTICLE_TYPE_LIST
#undef X
    COUNT
};
static const char* ParticleTypeNames[] = 
{
#define X(NAME) #NAME,
    PARTICLE_TYPE_LIST
#undef X
};

struct ParticleState
{
    ParticleType type;
    int life;
};
constexpr ParticleState defaultParticleState(ParticleType type)
{
    switch (type)
    {
    case ParticleType::Steam:
        return { .type = ParticleType::Steam, .life = 100 };

    default:
        return { .type = type, .life = 0 };

    }
}