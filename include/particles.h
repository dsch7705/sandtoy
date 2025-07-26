#pragma once


#define PARTICLE_TYPE_LIST \
    X(Air)                 \
    X(Sand)                \
    X(Gravel)              \
    X(Dirt)                \
    X(Stone)               \
    X(Rainbow)             \
    X(Pink)                \
    X(Blue)

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