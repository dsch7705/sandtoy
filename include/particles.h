#pragma once

#include <unordered_map>
#include <string>
#include <array>
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
const std::string kParticlePhaseNames[] = 
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
    X(White) \
    X(Red) \
    X(Orange) \
    X(Yellow) \
    X(Green) \
    X(Blue) \
    X(Purple) \
    X(Pink) \
    X(Rainbow) \
    X(Air) \
    X(Raulium)

enum class ParticleType
{
#define X(NAME) NAME,
    PARTICLE_LIST
#undef X
    COUNT
};
const std::string kParticleTypeNames[]
{
#define X(NAME) #NAME,
    PARTICLE_LIST
#undef X
};

typedef std::array<uint32_t, 5> ParticleColors;
const std::unordered_map<ParticleType, ParticleColors> kParticleColors {
    { ParticleType::Sand, { 0xE2C290FF, 0xD6B77EFF, 0xF0D8A8FF, 0xCCAA72FF, 0xB8935EFF } },
    { ParticleType::Stone, { 0x4A4A4AFF, 0x505050FF, 0x464646FF, 0x4C4C4CFF, 0x444444FF } },
    { ParticleType::Crucible, { 0x2A2A2A66, 0x2C2C2C66, 0x2E2E2E66, 0x31313166, 0x35353566 } },
    { ParticleType::Water, { 0x4DA6FF66, 0x4CA4F966, 0x4BA2F566, 0x4CA3FB66, 0x4EA7FD66 } },
    { ParticleType::Gravel, { 0x6A6A6AFF, 0x707070FF, 0x666666FF, 0x5E5E5EFF, 0x747474FF } },
    { ParticleType::Dirt, { 0x5A3A1EFF, 0x684425FF, 0x4E3018FF, 0x6F482BFF, 0x59391FFF } },
    { ParticleType::White, { 0xFFFFFFFF, 0xF5F5F5FF, 0xEAEAEAFF, 0xDDDDDDFF, 0xCCCCCCFF } },
    { ParticleType::Red, { 0xFF1A1AFF, 0xFF3333FF, 0xFF4D4DFF, 0xFF6666FF, 0xFF8080FF } },
    { ParticleType::Orange, { 0xFF6600FF, 0xFF751AFF, 0xFF8533FF, 0xFF944DFF, 0xFFA366FF } },
    { ParticleType::Yellow, { 0xFFD500FF, 0xFFDD22FF, 0xFFE347FF, 0xFFEA66FF, 0xFFF176FF } }, 
    { ParticleType::Green, { 0x00C46AFF, 0x00D47CFF, 0x00E68FFF, 0x1AF5A0FF, 0x4DFFB3FF } },
    { ParticleType::Blue, { 0x007BFFFF, 0x1A89FFFF, 0x3396FFFF, 0x4DA3FFFF, 0x66B0FFFF } },
    { ParticleType::Purple, { 0xB833FFFF, 0xC24DFFFF, 0xCC66FFFF, 0xD680FFFF, 0xE099FFFF } },
    { ParticleType::Pink, { 0xFF3FB8FF, 0xFF5CC2FF, 0xFF79CCFF, 0xFF96D6FF, 0xFFB3E0FF } },
    { ParticleType::Rainbow, { 0xEF476FFF, 0xFFA600FF, 0x06D6A0FF, 0x118AB2FF, 0x9B5DE5FF } },
    { ParticleType::Air, { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 } },
    { ParticleType::Raulium, { 0x006847FF, 0xFFFFFFFF, 0xCE1126FF, 0x8F4620FF, 0x0C8489FF } },

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

const std::unordered_map<ParticleType, ParticleProperties> kParticleProperties {
    { ParticleType::Sand, kSandProperties },
    { ParticleType::Stone, kStoneProperties },
    { ParticleType::Crucible, kCrucibleProperties },
    { ParticleType::Water, kWaterProperties },
    { ParticleType::Gravel, kSandProperties },
    { ParticleType::Dirt, kSandProperties },
    { ParticleType::White, kSandProperties },
    { ParticleType::Red, kSandProperties },
    { ParticleType::Orange, kSandProperties },
    { ParticleType::Yellow, kSandProperties },
    { ParticleType::Green, kSandProperties },
    { ParticleType::Blue, kSandProperties },
    { ParticleType::Purple, kSandProperties },
    { ParticleType::Pink, kSandProperties },
    { ParticleType::Rainbow, kSandProperties },
    { ParticleType::Air, kAirProperties },
    { ParticleType::Raulium, kSandProperties },
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
