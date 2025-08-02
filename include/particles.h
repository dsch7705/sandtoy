#pragma once

#include <unordered_map>
#include <string>


enum class ParticlePhase
{
    Solid,
    Liquid,
    Gas,
    Static
};
#define PARTICLE_LIST \
    X(Sand) \
    X(Stone) \
    X(Water) \
    X(Gravel) \
    X(Dirt) \
    X(Blue) \
    X(Pink) \
    X(Rainbow) \
    X(Air) \
    X(Steam)

enum class ParticleType
{
#define X(NAME) NAME,
    PARTICLE_LIST
#undef X
    COUNT
};
static constexpr std::string kParticleTypeNames[] = 
{
#define X(NAME) #NAME,
    PARTICLE_LIST
#undef X
};

struct ParticleProperties
{
    ParticlePhase phase;
    float specificHeat;
    float thermalConductivity;
    float meltingPoint;
    float boilingPoint;
    float latentHeatFusion;
    float latentHeatVaporization;
};
static constexpr ParticleProperties kSandProperties  { .phase = ParticlePhase::Solid, .specificHeat = .830f, .thermalConductivity = 0.25f };
static constexpr ParticleProperties kStoneProperties { .phase = ParticlePhase::Static, .specificHeat = .880f, .thermalConductivity = 2.3f };
static constexpr ParticleProperties kWaterProperties { .phase = ParticlePhase::Liquid, .specificHeat = 4.186f, .thermalConductivity = 0.6f };

static const std::unordered_map<ParticleType, ParticleProperties> kParticleProperties {
    { ParticleType::Sand, kSandProperties },
    { ParticleType::Stone, kStoneProperties },
    { ParticleType::Water, kWaterProperties },
    { ParticleType::Gravel, kSandProperties },
    { ParticleType::Dirt, kSandProperties },
    { ParticleType::Blue, kSandProperties },
    { ParticleType::Pink, kSandProperties },
    { ParticleType::Rainbow, kSandProperties },
    { ParticleType::Air, { .phase = ParticlePhase::Static, .specificHeat = 1.f, .thermalConductivity = 0.025f } },
    { ParticleType::Steam, { .phase = ParticlePhase::Gas, .specificHeat = 2.f, .thermalConductivity = 0.6f } }
};

struct ParticleState
{
    ParticleType type;
    ParticlePhase phase() const;

    float temperature;
    float temperatureDelta;
    float latentHeatStored;
    
    bool operator==(const ParticleState& other) const
    {
        return (type == other.type && temperature == other.temperature && temperatureDelta == other.temperatureDelta);
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