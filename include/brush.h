#include "particle_grid.h"

#include <stack>
#include <vector>


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
    float m_radius;
    bool m_isDown;
    
    ParticleType m_particleType;
    ParticleType m_particleType2;
    ParticleGrid* m_canvas;

    // Stores canvas states when edits are made
    std::stack<std::vector<ParticleType>> m_canvasStateStack;
    void pushCanvasState();
    void popCanvasState();

    std::vector<Cell*> m_selectedCells;
    void updateSelectedCells();

    friend class ParticleGrid;

};