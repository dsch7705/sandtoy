#include "particle_grid.h"

#include <stack>
#include <vector>
#include <unordered_set>


#define BRUSH_SHAPE_LIST \
    X(Circle) \
    X(Square)

enum class BrushType
{
#define X(NAME) NAME,
    BRUSH_SHAPE_LIST
#undef X
    COUNT
};
static constexpr const char* BrushTypeNames[] =
{
#define X(NAME) #NAME,
    BRUSH_SHAPE_LIST
#undef X
};

// Forward Declarations //
union SDL_Event;
//////////////////////////
class Brush
{
public:
    Brush(int radius, ParticleType particleType);

    void handleEvent(SDL_Event* event, bool isUiFocused);
    void update();

    void selectFill();
    void floodFill();

    void setCanvas(ParticleGrid* canvas);

    void setParticleType(ParticleType type);
    void setBrushType(BrushType type);    
    void setRadius(int radius);
    void setRotation(float radians);
    void setPos(int x, int y);
    
    ParticleType particleType() const;
    BrushType brushType() const;
    int radius() const;
    float rotation() const;

    void toggleHighlight();
    bool highlight() const;

    static constexpr int kMinRadius { 1 };
    static constexpr int kMaxRadius { 20 };
    // Scales the rate at which the scroll wheel resizes the brush
    static constexpr int kRadiusResizeScale { 1 };
    static constexpr float kRotationScale { 0.1f };

private:
    int m_x, m_y;
    int m_radius;
    float m_rot;
    bool m_isDown;
    
    ParticleType m_particleType;
    ParticleType m_particleType2;
    ParticleGrid* m_canvas;

    std::vector<Cell*> m_selectedCells;

    // Stores canvas states when edits are made
    struct CompoundState
    {
        ParticleState particleState;
        CellState cellState;
    };
    std::stack<std::vector<CompoundState>> m_canvasStateStack;
    void pushCanvasState();
    void popCanvasState();

    // Outline of the brush
    struct Shape
    {
        std::unordered_set<Cell*> outline;
        int x, y;
    };
    Shape m_shape;
    void setShapeCircle();
    void setShapeSquare();

    BrushType m_brushType;

    friend class ParticleGrid;

};