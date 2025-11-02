#include "particle_grid.h"
#include "particles.h"
#include "util.h"

#include "SDL3/SDL_surface.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <random>


const CellState kDefaultCellState(0.f, 0.f);


Cell::Cell(ParticleGrid* particleGrid, int _x, int _y, ParticleState particleState) 
    : x(_x), y(_y)
    , m_particleState(particleState)
    , m_needsRedraw(false)
    , m_isBrushSelected(false)
    , m_cellState(kDefaultCellState)
{
    if (particleGrid == nullptr)
    {
        throw std::runtime_error("particleGrid must not be null");
    }
    m_particleGrid = particleGrid;
    colorVariation = std::rand() % 5;
}
bool Cell::setParticleState(ParticleState state)
{
    if (state == m_particleState)
    {
        return false;
    }
    if (state.type != m_particleState.type || state.temperature != m_particleState.temperature)
    {
        markForRedraw();
    }
    m_particleState = state;
    return true;
}
ParticleState Cell::particleState() const
{
    return m_particleState;
}
void Cell::setCellState(CellState state)
{
    if (state == m_cellState)
    {
        return;
    }
    if (state.temperature != m_cellState.temperature)
    {
        markForRedraw();
    }
    m_cellState = state;
}
CellState Cell::cellState() const
{
    return m_cellState;
}

void Cell::setBrushSelected(bool selected)
{
    if (m_isBrushSelected != selected)
    {
        m_isBrushSelected = selected;
        markForRedraw();
    }
}
bool Cell::isBrushSelected() const
{
    return m_isBrushSelected;
}
void Cell::setBrushOutline(bool selected)
{
    if (m_isBrushOutline != selected)
    {
        m_isBrushOutline = selected;
        markForRedraw();
    }
}
bool Cell::isBrushOutline() const
{
    return m_isBrushOutline;
}

void Cell::markForRedraw()
{
    if (!m_needsRedraw)
    {
        m_particleGrid->m_redrawCells.push_back(this);
        m_needsRedraw = true;
    }
}

static SDL_Surface* sfMexicanFlag { nullptr };
ParticleGrid::ParticleGrid(const int w, const int h, SDL_Renderer* renderer)
    : width(w)
    , height(h)
{
    assert(w > 0 && "w must be greater than 0");
    assert(h > 0 && "h must be greater than 0");

    m_particles.reserve(width * height);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            m_particles.emplace_back(this, x, y, defaultParticleState(ParticleType::Air, ambientTemperature));
            m_coords.emplace_back(x, y);

            m_particles.back().markForRedraw();
        }
    }

    m_renderer = renderer;
    int rW, rH;
    SDL_GetCurrentRenderOutputSize(m_renderer, &rW, &rH);
    m_rendererRect = { .x = 0, .y = 0, .w = static_cast<float>(rW), .h = static_cast<float>(rH) };

    // Textures
    m_streamingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_SetTextureScaleMode(m_streamingTexture, SDL_SCALEMODE_NEAREST);

    if (sfMexicanFlag == nullptr)
    {
        SDL_Texture* texFull = IMG_LoadTexture(renderer, ASSETS_DIR"/images/mexico.png");
        assert(texFull != nullptr);

        SDL_Texture* texScaled = Util::scaleTexture(renderer, texFull, w, h);
        assert(texScaled != nullptr);

        SDL_Rect gridRect { 0, 0, w, h };
        SDL_SetRenderTarget(renderer, texScaled);
        SDL_Surface* sfARGB = SDL_RenderReadPixels(renderer, &gridRect);
        SDL_SetRenderTarget(renderer, nullptr);
        sfMexicanFlag = SDL_ConvertSurface(sfARGB, SDL_PIXELFORMAT_RGBA8888);

        SDL_DestroySurface(sfARGB);
        SDL_DestroyTexture(texFull);
        SDL_DestroyTexture(texScaled);
    }
}
ParticleGrid::~ParticleGrid()
{
    SDL_DestroyTexture(m_streamingTexture);
}
void ParticleGrid::startup()
{
    
}
void ParticleGrid::cleanup()
{
    SDL_DestroySurface(sfMexicanFlag);
}

Cell* ParticleGrid::getCell(int x, int y)
{
    if (y >= height || y < 0 || x >= width || x < 0)
    {
        return nullptr;
    }

    return &m_particles[y * width + x];
}

void ParticleGrid::draw()
{
    void* pixels;
    int pitch;
    SDL_LockTexture(m_streamingTexture, nullptr, &pixels, &pitch);
    if (pitch / sizeof(Uint32) != width)
    {
        std::cerr << "Streaming texture dimensions don't match those of the particle grid" << std::endl;
        return;
    }
    Uint32* pixelBuffer = static_cast<Uint32*>(pixels);

    for (Cell* cell : m_redrawCells)
    {
        Uint32 cellColor;
        constexpr Uint32 kErrorColor { 0xFF00FFFF };
        ParticleType type = cell->particleState().type;
        switch (type)
        {
        case ParticleType::Raulium:
        {
            if (sfMexicanFlag == nullptr || sfMexicanFlag->pixels == nullptr)
            {
                cellColor = kErrorColor;
                break;
            }

            Uint32* pixels = static_cast<Uint32*>(sfMexicanFlag->pixels);
            Uint32 flipped = pixels[cell->y * width + cell->x];
            //cellColor = ((flipped & 0x00FFFFFF) << 8) | ((flipped & 0xFF000000) >> 24); 
            cellColor = flipped;
            break;
        }
        default:
            cellColor = kParticleColors.at(type).at(cell->colorVariation);
            break;
        }
    
        // Blackbody radiation
        cellColor = Util::blendRGBA(cellColor, Util::temperatureToColor(cell->particleState().temperature, Util::TemperatureColorMode::Radiation));

        if (m_showTemperature) 
        { 
            cellColor = Util::blendRGBA(cellColor, Util::temperatureToColor(cell->particleState().temperature, m_tempColorMode));
        }

        // Add brush overlay
        if (m_showBrushHighlight && cell->isBrushSelected())
        {
            cellColor = Util::blendRGBA(cellColor, 0xFFFFFF22);
        }

        pixelBuffer[cell->y * (pitch / sizeof(Uint32)) + cell->x] = cellColor;
        cell->m_needsRedraw = false;
    }
    m_redrawCells.clear();
    
    SDL_UnlockTexture(m_streamingTexture);
    SDL_RenderTexture(m_renderer, m_streamingTexture, nullptr, &m_rendererRect);
}
void ParticleGrid::update()
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    
    if (isPaused)
        return;

    std::shuffle(m_coords.begin(), m_coords.end(), g);
    for (const std::pair<int, int>& coord : m_coords)
    {
        updateCell(coord.first, coord.second);
    }
    
    // Conductive heat model
    // Phase 1: accumulate deltas
    std::vector<float> accumulatedDelta(m_particles.size(), 0.f);

    const std::pair<int, int> neighborOffsets[] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
        //{1, 0}, {1, 1}, {0, 1}, {-1, 1}
    };

    for (const auto& coord : m_coords)
    {
        int x = coord.first;
        int y = coord.second;
        Cell* cell = getCell(x, y);
        if (!cell) continue;

        ParticleState a = cell->particleState();
        int idxA = y * width + x;

        for (const auto& offset : neighborOffsets)
        {
            int nx = x + offset.first;
            int ny = y + offset.second;

            float tempDiff;
            float delta;

            Cell* neighbor = getCell(nx, ny);
            if (neighbor)
            {
                ParticleState b = neighbor->particleState();
                int idxB = ny * width + nx;
            
                tempDiff = b.temperature - a.temperature;
                delta = tempDiff * 0.05f; 

                accumulatedDelta[idxA] += delta;
                accumulatedDelta[idxB] -= delta;
            }
            else
            {
                tempDiff = ambientTemperature - a.temperature;
                delta = tempDiff * 0.05f;

                accumulatedDelta[idxA] += delta;
            }
        }
    }

    // Phase 2: Apply accumulated deltas, finalize temps and reset deltas
    for (Cell& cell : m_particles)
    {
        ParticleState state = cell.particleState();
        const ParticleProperties& props = kParticleProperties.at(state.type);

        // Apply heat change
        state.temperatureDelta += accumulatedDelta[cell.y * width + cell.x];
        float heatEnergy = state.temperatureDelta;
        const float maxLatentTransferRate = 5.f;

        // --- SOLID TO LIQUID (MELTING) ---
        if (state.phase == ParticlePhase::Solid && state.temperature >= props.meltingPoint)
        {
            if (heatEnergy > 0) { // Particle is absorbing heat
                // How much latent heat do we still need to absorb to melt?
                float neededLatent = props.latentHeatFusion - state.latentHeatAbsorbed;
                // How much latent heat can we transfer this step?
                float actualLatentTransferred = std::min({heatEnergy, neededLatent, maxLatentTransferRate});

                state.latentHeatAbsorbed += actualLatentTransferred;
                state.temperature = props.meltingPoint; // Keep temp at melting point during phase change

                // Remove the transferred latent heat from heatEnergy, any remainder will be used for temperature change later
                heatEnergy -= actualLatentTransferred; // This is crucial for conservation

                if (state.latentHeatAbsorbed >= props.latentHeatFusion - 1e-6f) // Use epsilon for float comparison
                {
                    state.phase = ParticlePhase::Liquid;
                    state.latentHeatAbsorbed = 0.f; // Reset after complete phase change
                    // Any remaining heatEnergy should now go into heating the liquid
                    state.temperature += (heatEnergy / props.specificHeat); // Apply remaining heat to temperature
                }
            } else { // Solid at melting point, but losing heat. It should cool as a solid.
                state.temperature += state.temperatureDelta; // Allow it to cool below melting point
                state.latentHeatAbsorbed = 0.f; // Not in a latent heat process
            }
            state.temperatureDelta = 0.f; // Reset delta at end of block
        }
        // --- LIQUID TO GAS (VAPORIZATION) ---
        else if (state.phase == ParticlePhase::Liquid && state.temperature >= props.boilingPoint)
        {
            if (heatEnergy > 0) { // Particle is absorbing heat
                float neededLatent = props.latentHeatVaporization - state.latentHeatAbsorbed;
                float actualLatentTransferred = std::min({heatEnergy, neededLatent, maxLatentTransferRate});

                state.latentHeatAbsorbed += actualLatentTransferred;
                state.temperature = props.boilingPoint;

                heatEnergy -= actualLatentTransferred; // Remove transferred latent heat

                if (state.latentHeatAbsorbed >= props.latentHeatVaporization - 1e-6f)
                {
                    state.phase = ParticlePhase::Gas;
                    state.latentHeatAbsorbed = 0.f;
                    state.temperature += (heatEnergy / props.specificHeat); // Apply remaining heat to temperature
                }
            } else { // Liquid at boiling point, losing heat. Should condense or cool.
                state.temperature += state.temperatureDelta;
                state.latentHeatAbsorbed = 0.f;
            }
            state.temperatureDelta = 0.f;
        }
        // --- LIQUID TO SOLID (FREEZING) ---
        else if (state.phase == ParticlePhase::Liquid && state.temperature <= props.meltingPoint)
        {
            if (heatEnergy < 0) { // Particle is losing heat (freezing)
                // How much latent heat do we still need to release to freeze?
                // Note: state.latentHeatAbsorbed is negative here, so props.latentHeatFusion + state.latentHeatAbsorbed
                // (e.g., 100 + (-20)) means we still need to release 80.
                float neededToRelease = props.latentHeatFusion + state.latentHeatAbsorbed;
                // How much heat can we release this step? Use abs for comparison with maxLatentTransferRate
                float actualLatentTransferred = std::max(heatEnergy, -maxLatentTransferRate); // This is already negative

                // Ensure we don't 'over-release' more than what's needed for the phase change
                // Or, more simply, clamp the change itself.
                // If heatEnergy is -10 and maxLatent is 5, actualTransferred is -5.
                // If heatEnergy is -2 and maxLatent is 5, actualTransferred is -2.
                // We need to ensure we don't go past neededToRelease (negative value)
                actualLatentTransferred = std::max(actualLatentTransferred, -neededToRelease); // Clamp to not release too much past 0

                state.latentHeatAbsorbed += actualLatentTransferred; // Decreases (becomes more negative)
                state.temperature = props.meltingPoint; // Clamps temperature during freezing

                // Remaining heatEnergy is what wasn't used for latent heat. It's still negative.
                heatEnergy -= actualLatentTransferred; // This will become more negative (remaining energy to remove)

                if (state.latentHeatAbsorbed <= -props.latentHeatFusion + 1e-6f) // Use epsilon for float comparison
                {
                    state.phase = ParticlePhase::Solid;
                    state.latentHeatAbsorbed = 0.0f; // Reset after complete phase change
                    // Any remaining negative heatEnergy should now go into cooling the solid
                    state.temperature += (heatEnergy / props.specificHeat); // Apply remaining heat to temperature
                }
            } else { // Liquid at melting point, but gaining heat. Should warm or re-melt.
                state.temperature += state.temperatureDelta;
                state.latentHeatAbsorbed = 0.f;
            }
            state.temperatureDelta = 0.f;
        }
        // --- GAS TO LIQUID (CONDENSATION) ---
        else if (state.phase == ParticlePhase::Gas && state.temperature <= props.boilingPoint)
        {
            if (heatEnergy < 0) { // Particle is losing heat (condensing)
                float neededToRelease = props.latentHeatVaporization + state.latentHeatAbsorbed;
                float actualLatentTransferred = std::max(heatEnergy, -maxLatentTransferRate);
                actualLatentTransferred = std::max(actualLatentTransferred, -neededToRelease);

                state.latentHeatAbsorbed += actualLatentTransferred;
                state.temperature = props.boilingPoint;

                heatEnergy -= actualLatentTransferred; // Remaining negative heat

                if (state.latentHeatAbsorbed <= -props.latentHeatVaporization + 1e-6f)
                {
                    state.phase = ParticlePhase::Liquid;
                    state.latentHeatAbsorbed = 0.0f;
                    state.temperature += (heatEnergy / props.specificHeat); // Apply remaining heat to temperature
                }
            } else { // Gas at boiling point, but gaining heat. Should heat up.
                state.temperature += state.temperatureDelta;
                state.latentHeatAbsorbed = 0.f;
            }
            state.temperatureDelta = 0.f;
        }
        // --- NO PHASE CHANGE / DEFAULT TEMPERATURE UPDATE ---
        else
        {
            state.temperature += state.temperatureDelta;
            state.latentHeatAbsorbed = 0.f; // Only reset if NOT actively in a phase transition
            state.temperatureDelta = 0.f; // Always reset delta for next step
        }

        // Clamp temperature
        state.temperature = std::min(std::max(state.temperature, Util::kAbsZero), Util::kMaxTemp);
        cell.setParticleState(state);
    }
}
bool ParticleGrid::clear(ParticleType type)
{
    bool cellReset { false };
    for (Cell& cell : m_particles)
    {
        if (cell.setParticleState(defaultParticleState(type, ambientTemperature)))
            cellReset = true;
    }

    return cellReset;
}
void ParticleGrid::toggleShowTemp()
{
    m_showTemperature = !m_showTemperature;
    for (Cell& cell : m_particles)
    {
        cell.markForRedraw();
    }
}
bool ParticleGrid::showTemp() const
{
    return m_showTemperature;
}
void ParticleGrid::setTempColorMode(Util::TemperatureColorMode mode) 
{
    if (mode == m_tempColorMode)
    {
        return;
    }

    m_tempColorMode = mode;
    for (Cell& cell : m_particles)
    {
        cell.markForRedraw();
    }
}
Util::TemperatureColorMode ParticleGrid::tempColorMode() const 
{
    return m_tempColorMode;
}

ParticleGrid::CanvasState ParticleGrid::getCanvasState() const
{
    CanvasState canvasState;
    canvasState.states.reserve(width * height);
    for (const Cell& cell : m_particles)
    {
        canvasState.states.emplace_back(cell.particleState(), cell.cellState());
    }
    return canvasState;
}
void ParticleGrid::setCanvasState(const CanvasState& state)
{
    size_t canvasSize = width * height;
    if (state.states.size() == canvasSize)
    {
        for (int i = 0; i < canvasSize; ++i)
        {
            Cell& cell = m_particles.at(i);
            CompoundState compoundState = state.states.at(i);
            cell.setParticleState(compoundState.particleState);
            cell.setCellState(compoundState.cellState);
        }

        isPaused = true;
    }
    else
    {
        std::cerr << __func__ << ": Canvas state size (" << state.states.size() << ") does not match current canvas size (" << canvasSize << " [" << width << "x" << height << "]); discarding\n";
    }
}

void ParticleGrid::updateCell(int x, int y)
{
    Cell* cell = getCell(x, y);
    if (cell == nullptr)
    {
        return;
    }

    // Positioning
    ParticleUpdate update = { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
    switch (cell->particleState().phase)
    {
    case ParticlePhase::Solid:
        update = particleUpdateFunc_Solid(this, x, y);
        break;
    
    case ParticlePhase::Liquid:
        update = particleUpdateFunc_Liquid(this, x, y);
        break;
    
    case ParticlePhase::Gas:
        update = particleUpdateFunc_Gas(this, x, y);
        break;

    case ParticlePhase::Static:
        update = particleUpdateFunc_Static(this, x, y);
        break;

    default:
        break;

    }

    switch (update.mode)
    {
    case ParticleUpdate::Move:
        update.nextCell->setParticleState(cell->particleState());
        cell->setParticleState({ .type = ParticleType::Air, .temperature = 0 });
        break;

    case ParticleUpdate::Swap:
    {
        ParticleState tmp = cell->particleState();
        cell->setParticleState(update.nextCell->particleState());
        update.nextCell->setParticleState(tmp);
        break;
    }

    case ParticleUpdate::NOOP:
    default:
        break;

    }
}
void ParticleGrid::update_b2t()
{
    for (int y = height; y >= 0; --y)
    {
        int scanDir = (y % 2) ? 1 : -1;
        int startX = (scanDir == 1) ? 0 : width;
        int endX = (startX == 0) ? width : -1;

        int x = startX;
        while (x != endX)
        {
            Cell* cell = getCell(x, y);
            if (cell != nullptr)
            {
                updateCell(cell->x, cell->y);
            }

            x += scanDir;
        }
    }
}
void ParticleGrid::update_t2b()
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Cell* cell = getCell(x, y);
            if (cell != nullptr)
            {
                updateCell(cell->x, cell->y);
            }
        }
    }
}
