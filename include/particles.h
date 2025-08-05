#pragma once

#include <unordered_map>
#include <string>
#include "util.h"


#define PARTICLE_PHASE_LIST \
    X(Solid) \
    X(Liquid) \
    X(Gas) \
    X(Static)

enum class ParticlePhase
{
#define X(NAME) NAME,
    PARTICLE_PHASE_LIST
#undef X
};
constexpr std::string kParticlePhaseNames[] = 
{
#define X(NAME) #NAME,
    PARTICLE_PHASE_LIST
#undef X
};
#define PARTICLE_LIST \
    X(Sand) \
    X(Stone) \
    X(Crucible) \
    X(Water) \
    X(Gravel) \
    X(Dirt) \
    X(Blue) \
    X(Pink) \
    X(Rainbow) \
    X(Air) 

enum class ParticleType
{
#define X(NAME) NAME,
    PARTICLE_LIST
#undef X
    COUNT
};
constexpr std::string kParticleTypeNames[] = 
{
#define X(NAME) #NAME,
    PARTICLE_LIST
#undef X
};

struct ParticleProperties
{
    float specificHeat;
    float thermalConductivity;
    float meltingPoint;
    float boilingPoint;
    float latentHeatFusion;
    float latentHeatVaporization;

    bool affectedByGravity { true };
    float density;
};
constexpr ParticleProperties kSandProperties  {
    .specificHeat = 0.20f,             
    .thermalConductivity = 0.00025f,    
    .meltingPoint = 1700.f,
    .boilingPoint = 2200.f,
    .latentHeatFusion = 1.9f,          
    .latentHeatVaporization = 4.5f,
    
    .density = 1.675f
};
constexpr ParticleProperties kStoneProperties {
    .specificHeat = 0.19f,             
    .thermalConductivity = 0.0015f,     
    .meltingPoint = 1260.f,
    .boilingPoint = 2600.f,            
    .latentHeatFusion = 1.5f,          
    .latentHeatVaporization = 4.0f,  

    .affectedByGravity = false,
    .density = 3.5f
};
constexpr ParticleProperties kCrucibleProperties {
    .specificHeat = 1.f,             
    .thermalConductivity = 0.0015f,     
    .meltingPoint = Util::kMaxTemp + 1.f,
    .boilingPoint = Util::kMaxTemp * 2.f,            
    .latentHeatFusion = 1.5f,          
    .latentHeatVaporization = 4.0f,  

    .affectedByGravity = false,
    .density = 3.5f
};
constexpr ParticleProperties kWaterProperties {
    .specificHeat = 1.00f,
    .thermalConductivity = 0.0006f,
    .meltingPoint = 0.f,
    .boilingPoint = 100.f,
    .latentHeatFusion = 0.33f,       
    .latentHeatVaporization = 2.26f,
    
    .density = 0.997f
};
constexpr ParticleProperties kAirProperties {
    .specificHeat = 0.24f,             
    .thermalConductivity = 0.00005f,    
    .meltingPoint = -218.f,            
    .boilingPoint = -194.f,            
    .latentHeatFusion = 0.3f,
    .latentHeatVaporization = 2.28f,

    .density = 0.0012f
};

static const std::unordered_map<ParticleType, ParticleProperties> kParticleProperties {
    { ParticleType::Sand, kSandProperties },
    { ParticleType::Stone, kStoneProperties },
    { ParticleType::Crucible, kCrucibleProperties },
    { ParticleType::Water, kWaterProperties },
    { ParticleType::Gravel, kSandProperties },
    { ParticleType::Dirt, kSandProperties },
    { ParticleType::Blue, kSandProperties },
    { ParticleType::Pink, kSandProperties },
    { ParticleType::Rainbow, kSandProperties },
    { ParticleType::Air, kAirProperties },
};

struct ParticleState
{
    ParticleType type;
    ParticlePhase phase;

    float temperature;
    float temperatureDelta;
    float latentHeatAbsorbed;
    
    bool operator==(const ParticleState& other) const
    {
        return (type == other.type && temperature == other.temperature && temperatureDelta == other.temperatureDelta);
    }
};
static ParticlePhase getParticlePhase(ParticleType type, float temperature)
{
    const ParticleProperties& props = kParticleProperties.at(type);
    if (temperature < props.meltingPoint)
    {
        return ParticlePhase::Solid;
    }
    else if (temperature >= props.meltingPoint && temperature < props.boilingPoint)
    {
        return ParticlePhase::Liquid;
    }
    else
    {
        return ParticlePhase::Gas;
    }
}
static const ParticleState defaultParticleState(ParticleType type, float temperature)
{
    return { .type = type, .phase = getParticlePhase(type, temperature), .temperature = temperature, .temperatureDelta = 0.f, .latentHeatAbsorbed = 0.f };
}