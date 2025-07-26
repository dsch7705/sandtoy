#include "particle_grid.h"


// Forward Declarations //
union SDL_Event;
//////////////////////////
class Brush
{
public:
    Brush(float radius, ParticleType particleType);

    void handleEvent(SDL_Event* event);

    void setParticleType(ParticleType type);
    ParticleType particleType() const;

    void setCanvas(ParticleGrid* canvas);

private:
    int m_x, m_y;
    bool m_isDown;
    
    float m_radius;
    ParticleType m_particleType;

    ParticleGrid* m_canvas;

};