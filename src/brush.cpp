#include "brush.h"

#include <SDL3/SDL.h>
#include <cmath>
#include <iostream>


Brush::Brush(float radius, ParticleType particleType)
    : m_x(0)
    , m_y(0)
    , m_isDown(false)
    , m_radius(radius)
    , m_particleType(particleType)
    , m_particleType2(ParticleType::Vacuum)
    , m_canvas(nullptr)
{

}

void Brush::handleEvent(SDL_Event* event, bool isUiFocused)
{
    switch (event->type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        switch (event->button.button)
        {
        case 1:
            if (!isUiFocused)
            {
                m_isDown = true;
            }
            break;

        case 3:
        {
            ParticleType tmp = m_particleType;
            m_particleType = m_particleType2;
            m_particleType2 = tmp;

            if (!isUiFocused)
            {
                m_isDown = true;
            }
            break;
        }

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

        case 3:
        {
            ParticleType tmp = m_particleType;
            m_particleType = m_particleType2;
            m_particleType2 = tmp;

            m_isDown = false;
            break;
        }

        default:
            break;

        }
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        if (SDL_GetModState() & SDL_KMOD_LCTRL)
        {
            int nextIdx = (static_cast<int>(m_particleType) + static_cast<int>(event->wheel.y) + static_cast<int>(ParticleType::COUNT)) 
                            % static_cast<int>(ParticleType::COUNT);
            m_particleType = static_cast<ParticleType>(nextIdx);
        }
        else
        {
            setRadius(m_radius + (kRadiusResizeScale * event->wheel.y));
            if      (m_radius > kMaxRadius) { m_radius = kMaxRadius; }
            else if (m_radius < kMinRadius) { m_radius = kMinRadius; }
        }
        break;

    case SDL_EVENT_MOUSE_MOTION:
        int x = (event->motion.x / m_canvas->m_rendererRect.w) * m_canvas->width();
        int y = (event->motion.y / m_canvas->m_rendererRect.h) * m_canvas->height();
        if (x != m_x || y != m_y)
        {
            updateSelectedCells();
        }

        m_x = x;
        m_y = y;
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
void Brush::setRadius(float radius)
{
    m_radius = radius;
    updateSelectedCells();
}
float Brush::radius() const
{
    return m_radius;
}

void Brush::updateSelectedCells()
{
    for (Cell* cell : m_selectedCells)
    {
        cell->setSelected(false);
    }
    m_selectedCells.clear();

    if (m_canvas == nullptr)
    {
        std::cerr << "No canvas has been set; Brush::setCanvas" << std::endl;
        return;
    }

    const int rowBegin = std::max(0, (int)(m_y - m_radius));
    const int rowEnd   = std::min(m_canvas->height() - 1, (int)(m_y + m_radius));

    const int colBegin = std::max(0, (int)(m_x - m_radius));
    const int colEnd = std::min(m_canvas->width() - 1, (int)(m_x + m_radius));

    for (int y = rowBegin; y <= rowEnd; ++y)
    {
        for (int x = colBegin; x <= colEnd; ++x)
        {
            float dist = std::sqrt(std::pow(y - m_y, 2.f) + std::pow(x - m_x, 2.f));
            if (dist <= m_radius)
            {
                Cell* cell = m_canvas->getCell(x, y);
                cell->setSelected(true);
                m_selectedCells.push_back(cell);
            }
        }        
    }
}
void Brush::stroke()
{
    if (m_isDown)
    {
        for (Cell* cell : m_selectedCells)
        {
            cell->setParticleType(m_particleType);
        }
    }
}