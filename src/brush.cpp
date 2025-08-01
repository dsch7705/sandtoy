#include "brush.h"
#include "util.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_set>


Brush::Brush(int radius, ParticleType particleType)
    : m_x(0)
    , m_y(0)
    , m_isDown(false)
    , m_radius(radius)
    , m_brushType(BrushType::Circle)
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
        //if ((SDL_GetModState() & (SDL_KMOD_SHIFT | SDL_KMOD_CTRL)) == (SDL_KMOD_SHIFT | SDL_KMOD_CTRL))
        //{
        //    int nextIdx = (static_cast<int>(m_brushType) + static_cast<int>(event->wheel.y) + static_cast<int>(BrushType::COUNT)) 
        //                    % static_cast<int>(BrushType::COUNT);
        //    setBrushType(static_cast<BrushType>(nextIdx));
        //}
        if (SDL_GetModState() & SDL_KMOD_CTRL)
        {
            int nextIdx = (static_cast<int>(m_particleType) + static_cast<int>(event->wheel.y) + static_cast<int>(ParticleType::COUNT)) 
                            % static_cast<int>(ParticleType::COUNT);
            m_particleType = static_cast<ParticleType>(nextIdx);
        }
        else if (SDL_GetModState() & SDL_KMOD_SHIFT)
        {
            float newRot = m_rot + (kRotationScale * event->wheel.y);
            if      (newRot > 2 * 3.14159f) { newRot = 2 * Util::PI; }
            else if (newRot < 0.f) { newRot = 0.f; }
            setRotation(newRot);
        }
        else
        {
            int newRadius = m_radius + (kRadiusResizeScale * event->wheel.y);
            if      (newRadius > kMaxRadius) { newRadius = kMaxRadius; }
            else if (newRadius < kMinRadius) { newRadius = kMinRadius; }
            setRadius(newRadius);
        }
        break;

    case SDL_EVENT_MOUSE_MOTION:
    {
        int x = (event->motion.x / m_canvas->m_rendererRect.w) * m_canvas->width;
        int y = (event->motion.y / m_canvas->m_rendererRect.h) * m_canvas->height;
        if (x != m_x || y != m_y)
        {
            setPos(x, y);
        }
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

void Brush::setCanvas(ParticleGrid* canvas)
{
    m_canvas = canvas;
}

void Brush::setParticleType(ParticleType type)
{
    m_particleType = type;
}
void Brush::setBrushType(BrushType type)
{
    if (m_brushType == type) { return; }

    m_brushType = type;
    switch (type)
    {
    default:
    case BrushType::Circle:
        setShapeCircle();
        break;

    case BrushType::Square:
        setShapeSquare();
        break;
    }
}
void Brush::setRadius(int radius)
{
    if (m_radius == radius) { return; }

    m_radius = radius;
    switch (m_brushType)
    {
    default:
    case BrushType::Circle:
        setShapeCircle();
        break;

    case BrushType::Square:
        setShapeSquare();
        break;
    }
}
void Brush::setRotation(float radians)
{
    if (m_rot == radians) { return; }

    m_rot = radians;
    switch (m_brushType)
    {
    default:
    case BrushType::Circle:
        break;

    case BrushType::Square:
        setShapeSquare();
        break;
    }
}
void Brush::setPos(int x, int y)
{
    m_x = x;
    m_y = y;
    switch (m_brushType)
    {
    default:
    case BrushType::Circle:
        setShapeCircle();
        break;

    case BrushType::Square:
        setShapeSquare();
        break;
    }
}

ParticleType Brush::particleType() const
{
    return m_particleType;
}
BrushType Brush::brushType() const
{
    return m_brushType;
}
int Brush::radius() const
{
    return m_radius;
}
float Brush::rotation() const
{
    return m_rot;
}

void Brush::toggleHighlight()
{
    m_canvas->m_showBrushHighlight = !m_canvas->m_showBrushHighlight;
    for (Cell* cell : m_shape.outline)
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

void Brush::setShapeCircle()
{
    for (Cell* cell : m_shape.outline)
    {
        cell->setBrushOutline(false);
    }
    m_shape.outline.clear();
    m_shape.x = m_x;
    m_shape.y = m_y;

    int x = 0;
    int y = m_radius;
    int d = 1 - y;  // Decision variable

    while (x <= y) 
    {
        auto tryPush = [&](int x, int y) {
            Cell* c = m_canvas->getCell(x, y);
            if (c) 
            { 
                c->setBrushOutline(true);
                m_shape.outline.insert(c);
            }
        };

        tryPush(m_x + x, m_y + y);
        tryPush(m_x - x, m_y + y);
        tryPush(m_x + x, m_y - y);
        tryPush(m_x - x, m_y - y);
        tryPush(m_x + y, m_y + x);
        tryPush(m_x - y, m_y + x);
        tryPush(m_x + y, m_y - x);
        tryPush(m_x - y, m_y - x);

        if (d < 0) 
        {
            d += 2 * x + 3;
        } 
        else 
        {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }

    selectFill();
}
void Brush::setShapeSquare()
{
    for (Cell* cell : m_shape.outline)
    {
        cell->setBrushOutline(false);
    }
    m_shape.outline.clear();
    m_shape.x = m_x;
    m_shape.y = m_y;

    auto tryPush = [&](int x, int y) {
        Cell* c = m_canvas->getCell(x, y);
        if (c) 
        { 
            c->setBrushOutline(true);
            m_shape.outline.insert(c);
        }
    };

    static const std::vector<std::pair<int, int>> vertices = {
        { 1, 1 },
        { -1, 1 },
        { -1, -1 },
        { 1, -1 }
    };
    static const int nVertices = vertices.size();

    for (int i = 0; i < nVertices; ++i)
    {
        auto [vx, vy] = vertices[i];
        auto [nx, ny] = vertices[(i + 1) % nVertices];

        // Rotate and scale first point
        float x1f = vx * std::cos(m_rot) - vy * std::sin(m_rot);
        float y1f = vx * std::sin(m_rot) + vy * std::cos(m_rot);
        x1f *= m_radius;
        y1f *= m_radius;

        // Rotate and scale next point
        float x2f = nx * std::cos(m_rot) - ny * std::sin(m_rot);
        float y2f = nx * std::sin(m_rot) + ny * std::cos(m_rot);
        x2f *= m_radius;
        y2f *= m_radius;

        // Translate and round to nearest integer pixel
        int x1 = static_cast<int>(std::round(m_shape.x + x1f));
        int y1 = static_cast<int>(std::round(m_shape.y + y1f));
        int x2 = static_cast<int>(std::round(m_shape.x + x2f));
        int y2 = static_cast<int>(std::round(m_shape.y + y2f));

        Util::bresenhamLine(x1, y1, x2, y2, [&](int x, int y) {
            tryPush(x, y);
        });
    }

    selectFill();
}

void Brush::update()
{
    if (m_isDown)
    {
        for (Cell* cell : m_selectedCells)
        {
            cell->setParticleState(defaultParticleState(m_particleType));
        }
    }
}

void Brush::selectFill()
{
    for (Cell* cell : m_selectedCells)
    {
        cell->setBrushSelected(false);
    }
    m_selectedCells.clear();

    std::queue<Cell*> q;
    std::unordered_set<Cell*> visited;
    Cell* cell = m_canvas->getCell(m_shape.x, m_shape.y);
    if (cell == nullptr)
    {
        return;
    }
    q.push(cell);

    while (!q.empty())
    {
        cell = q.front();
        q.pop();
        if (visited.contains(cell) || cell->isBrushOutline()) continue;

        cell->setBrushSelected(true);
        m_selectedCells.insert(cell);
        visited.insert(cell);

        auto tryPush = [&](int x, int y) {
            Cell* c = m_canvas->getCell(x, y);
            if (c) { q.push(c); }
        };

        tryPush(cell->x + 1, cell->y);
        tryPush(cell->x - 1, cell->y);
        tryPush(cell->x, cell->y + 1);
        tryPush(cell->x, cell->y - 1);
    }
}
void Brush::floodFill()
{
    std::queue<Cell*> q;
    Cell* cell = m_canvas->getCell(m_x, m_y);
    if (cell == nullptr)
    {
        std::cerr << "Invalid brush position [" << m_x << ", " << m_y << "]\n";
        return;
    }
    q.push(cell);
    ParticleType target = cell->particleState().type;

    while (!q.empty())
    {
        cell = q.front();
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