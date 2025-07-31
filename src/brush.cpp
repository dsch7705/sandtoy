#include "brush.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>
#include <queue>


Brush::Brush(float radius, ParticleType particleType)
    : m_x(0)
    , m_y(0)
    , m_isDown(false)
    , m_radius(radius)
    , m_particleType(particleType)
    , m_particleType2(ParticleType::Air)
    , m_canvas(nullptr)
{

}

void Brush::handleEvent(SDL_Event* event, bool isUiFocused)
{
    switch (event->type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        pushCanvasState();
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
        if (SDL_GetModState() & SDL_KMOD_CTRL)
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
    {
        int x = (event->motion.x / m_canvas->m_rendererRect.w) * m_canvas->width;
        int y = (event->motion.y / m_canvas->m_rendererRect.h) * m_canvas->height;
        if (x != m_x || y != m_y)
        {
            updateSelectedCells();
        }

        m_x = x;
        m_y = y;
        break;
    }

    case SDL_EVENT_KEY_DOWN:
        switch (event->key.key)
        {
        case SDLK_F:
            if (event->key.mod & SDL_KMOD_ALT) break;
            pushCanvasState();
            floodFill();
            break;

        case SDLK_Z:
            if (event->key.mod & SDL_KMOD_CTRL)
            {
                popCanvasState();
            }

        }

    default:
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

void Brush::toggleHighlight()
{
    m_canvas->m_showBrushHighlight = !m_canvas->m_showBrushHighlight;
    for (Cell* cell : m_selectedCells)
    {
        cell->markForRedraw();
    }
}
bool Brush::highlight() const
{
    return m_canvas->m_showBrushHighlight;
}

void Brush::pushCanvasState()
{
    std::vector<ParticleState> state;
    state.reserve(m_canvas->width * m_canvas->height);
    for (const Cell& cell : m_canvas->m_particles)
    {
        state.push_back(cell.particleState());
    }
    m_canvasStateStack.push(std::move(state));
}
void Brush::popCanvasState()
{
    if (m_canvasStateStack.empty()) return;

    size_t canvasSize = m_canvas->width * m_canvas->height;
    std::vector<ParticleState>& state = m_canvasStateStack.top();
    if (state.size() == canvasSize)
    {
        //int y = 0;
        //for (std::vector<Cell>& row : m_canvas->m_particles)
        //{
        //    int x = 0;
        //    for (Cell& cell : row)
        //    {
        //        cell.setParticleState(state[y * row.size() + x]);
        //        ++x;
        //    }
        //    ++y;
        //}
        int i = 0;
        for (Cell& cell : m_canvas->m_particles)
        {
            cell.setParticleState(state[i]);
            ++i;
        }
    }
    else
    {
        std::cerr << __func__ << ": Canvas state size (" << state.size() << ") does not match current canvas size (" << canvasSize << " [" << m_canvas->width << "x" << m_canvas->height << "]); discarding\n";
    }
    m_canvasStateStack.pop();
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
    const int rowEnd   = std::min(m_canvas->height - 1, (int)(m_y + m_radius));

    const int colBegin = std::max(0, (int)(m_x - m_radius));
    const int colEnd = std::min(m_canvas->width - 1, (int)(m_x + m_radius));

    for (int y = rowBegin; y <= rowEnd; ++y)
    {
        for (int x = colBegin; x <= colEnd; ++x)
        {
            float distSqr = (y - m_y) * (y - m_y) + (x - m_x) * (x - m_x);
            //float dist = std::sqrt(std::pow(y - m_y, 2.f) + std::pow(x - m_x, 2.f));
            if (distSqr <= m_radius * m_radius)
            //if (dist <= m_radius)
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
            cell->setParticleState(defaultParticleState(m_particleType));
        }
    }
}
void Brush::floodFill()
{
    std::queue<Cell*> q;
    q.push(m_canvas->getCell(m_x, m_y));
    ParticleType target = q.back()->particleState().type;

    while (!q.empty())
    {
        Cell* cell = q.front();
        q.pop();
        if (cell == nullptr || cell->particleState().type == m_particleType) continue;

        if (cell->particleState().type == target)
        {
            cell->setParticleState(defaultParticleState(m_particleType));
            q.push(m_canvas->getCell(cell->x + 1, cell->y));
            q.push(m_canvas->getCell(cell->x - 1, cell->y));
            q.push(m_canvas->getCell(cell->x, cell->y + 1));
            q.push(m_canvas->getCell(cell->x, cell->y - 1));
        }
    }
}