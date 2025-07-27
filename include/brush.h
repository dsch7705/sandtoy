#include "particle_grid.h"


// Forward Declarations //
union SDL_Event;
//////////////////////////
class Brush
{
public:
    Brush(float radius, ParticleType particleType);

    void handleEvent(SDL_Event* event, bool isUiFocused);
    void stroke();
    void floodFill();

    void setParticleType(ParticleType type);
    ParticleType particleType() const;

    void setCanvas(ParticleGrid* canvas);
    void setRadius(float radius);
    float radius() const;

    void toggleHighlight();
    bool highlight() const;

    static constexpr float kMinRadius { 0.5f };
    static constexpr float kMaxRadius { 100.f };
    // Scales the rate at which the scroll wheel resizes the brush
    static constexpr float kRadiusResizeScale { 0.5f };

private:
    int m_x, m_y;
    bool m_isDown;
    
    float m_radius;
    ParticleType m_particleType;
    ParticleType m_particleType2;

    ParticleGrid* m_canvas;

    std::vector<Cell*> m_selectedCells;
    void updateSelectedCells();

    friend class ParticleGrid;

};