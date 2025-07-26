#include "brush.h"

#include <SDL3/SDL.h>


Brush::Brush(float radius, ParticleType particleType)
    : m_x(0)
    , m_y(0)
    , m_isDown(false)
    , m_radius(radius)
    , m_particleType()
{

}

void Brush::handleEvent(SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        switch (event->button.button)
        {
        case 1:
            m_isDown = true;
            break;

        default:
            break;

        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        switch (event->button.button)
        {
        case 1:
            m_isDown = false;
            break;

        default:
            break;

        }
        break;

    case SDL_EVENT_MOUSE_MOTION:
        m_x = event->motion.x;
        m_y = event->motion.y;
        break;

    }
}

void Brush::setParticleType(ParticleType type)
{
    m_particleType = type;
}
ParticleType Brush::particleType() const
{
    return m_particleType;
}

void Brush::setCanvas(ParticleGrid* canvas)
{
    m_canvas = canvas;
}