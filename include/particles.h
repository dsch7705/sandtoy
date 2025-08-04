#pragma once

#include <unordered_map>
#include <string>
#include <stdexcept>


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

constexpr float kAbsZero { -273.15f };
constexpr float kMaxTemp { 3000.f };
struct ParticleProperties
{
    float specificHeat;
    float thermalConductivity;
    float meltingPoint;
    float boilingPoint;
    float latentHeatFusion;
    float latentHeatVaporization;

    bool affectedByGravity { true };
};
constexpr ParticleProperties kSandProperties  {
    .specificHeat = 0.20f,             
    .thermalConductivity = 0.00025f,    
    .meltingPoint = 1700.f,
    .boilingPoint = 2200.f,
    .latentHeatFusion = 1.9f,          
    .latentHeatVaporization = 4.5f     
};
constexpr ParticleProperties kStoneProperties {
    .specificHeat = 0.19f,             
    .thermalConductivity = 0.0015f,     
    .meltingPoint = 1260.f,
    .boilingPoint = 2600.f,            
    .latentHeatFusion = 1.5f,          
    .latentHeatVaporization = 4.0f,    
    .affectedByGravity = false
};
constexpr ParticleProperties kWaterProperties {
    .specificHeat = 1.00f,
    .thermalConductivity = 0.0006f,
    .meltingPoint = 0.f,
    .boilingPoint = 100.f,
    .latentHeatFusion = 0.33f,       
    .latentHeatVaporization = 2.26f  
};
constexpr ParticleProperties kAirProperties {
    .specificHeat = 0.24f,             
    .thermalConductivity = 0.00005f,    
    .meltingPoint = -218.f,            
    .boilingPoint = -194.f,            
    .latentHeatFusion = 0.3f,
    .latentHeatVaporization = 2.28f
};

static const std::unordered_map<ParticleType, ParticleProperties> kParticleProperties {
    { ParticleType::Sand, kSandProperties },
    { ParticleType::Stone, kStoneProperties },
    { ParticleType::Water, kWaterProperties },
    { ParticleType::Gravel, kSandProperties },
    { ParticleType::Dirt, kSandProperties },
    { ParticleType::Blue, kSandProperties },
    { ParticleType::Pink, kSandProperties },
    { ParticleType::Rainbow, kSandProperties },
    { ParticleType::Air, kAirProperties },
};
inline static const std::runtime_error errParticlePropertiesNotFound(ParticleType type)
{
    return std::runtime_error("No entry for ParticleType '" + kParticleTypeNames[static_cast<int>(type)] + "' " + "in kParticleProperties");
}

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
    if (!kParticleProperties.contains(type))
    {
        throw errParticlePropertiesNotFound(type);
    }

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