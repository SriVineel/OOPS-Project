//=============================================================================
// BrushStroke.cpp
//=============================================================================
// PURPOSE:
//   Implements the freehand brush stroke data structure. Stores sampled
//   points and provides drawing utilities used by the canvas.
//
// KEY NOTES:
//   - Points are stored as absolute world coordinates.
//   - `addPoint` interpolates between samples to avoid gaps when the mouse
//     moves quickly.
//   - Drawing renders filled circles for each sample to approximate stroke
//     thickness.
//=============================================================================

#include "BrushStroke.h"

#include <algorithm>
#include <cmath>

BrushStroke::BrushStroke(const std::string& id,
                         const sf::Color& color,
                         float thickness)
    : CanvasObject(id, 0.f, 0.f, 0.f, 0.f, 0.f),
      m_vertices(sf::PrimitiveType::LineStrip),
      color_(color),
      thickness_(thickness) // currently not used (SFML lines are 1px)
{
}

void BrushStroke::beginAt(const sf::Vector2f& pos)
{
    m_vertices.clear();

    // First point defines initial bounds
    sf::Vertex v;
    v.position = pos;
    v.color    = color_;
    m_vertices.append(v);

    x_ = pos.x;
    y_ = pos.y;
    width_  = 0.f;
    height_ = 0.f;

    // Keep SFML-side state consistent with logical state
    m_position = {x_, y_};
    m_size     = {width_, height_};
}

void BrushStroke::addPoint(const sf::Vector2f& pos)
{
    // If there's no previous point, just add this one
    if (m_vertices.getVertexCount() == 0)
    {
        sf::Vertex v;
        v.position = pos;
        v.color    = color_;
        m_vertices.append(v);
        updateBoundsForPoint(pos);
        return;
    }

    // Interpolate between last point and this point so fast mouse moves
    // don't produce gaps. Use spacing guided by stroke thickness.
    auto &lastV = m_vertices[m_vertices.getVertexCount() - 1];
    sf::Vector2f lastPos = lastV.position;

    float dx = pos.x - lastPos.x;
    float dy = pos.y - lastPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    // spacing in pixels between generated points (denser for smoother appearance)
    float spacing = thickness_ * 0.5f;
    spacing = std::max(spacing, 0.5f);  // Ensure minimum 0.5 px spacing

    if (dist <= spacing)
    {
        sf::Vertex v;
        v.position = pos;
        v.color    = color_;
        m_vertices.append(v);
        updateBoundsForPoint(pos);
        return;
    }

    int steps = static_cast<int>(std::ceil(dist / spacing));
    for (int i = 1; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        sf::Vertex v;
        v.position = sf::Vector2f(lastPos.x + t * dx, lastPos.y + t * dy);
        v.color = color_;
        m_vertices.append(v);
        updateBoundsForPoint(v.position);
    }
}

void BrushStroke::setColor(const sf::Color& c)
{
    color_ = c;
    // Update existing vertices to new color
    for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i)
    {
        m_vertices[i].color = color_;
    }
}

sf::Color BrushStroke::getColor() const
{
    return color_;
}

void BrushStroke::draw(sf::RenderWindow& window) {
    if (m_vertices.getVertexCount() == 0)
        return;

    // Use the stored thickness_ member
    float radius = std::max(thickness_ * 0.5f, 0.5f);
    sf::CircleShape dot(radius);

    // SFML 3: setOrigin(Vector2f origin)
    dot.setOrigin(sf::Vector2f(radius, radius));

    for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i) {
        dot.setPosition(m_vertices[i].position);
        dot.setFillColor(color_);
        window.draw(dot);
    }
}


bool BrushStroke::isClicked(float mouseX, float mouseY) const
{
    // Basic hit test: inside logical bounding box.
    // You can refine this later to check distance to the polyline.
    bool inX = (mouseX >= x_) && (mouseX <= x_ + width_);
    bool inY = (mouseY >= y_) && (mouseY <= y_ + height_);
    return inX && inY;
}

void BrushStroke::updateBoundsForPoint(const sf::Vector2f& p)
{
    // Compute min/max using current bounds and the new point
    float minX = std::min(x_, p.x);
    float minY = std::min(y_, p.y);
    float maxX = std::max(x_ + width_,  p.x);
    float maxY = std::max(y_ + height_, p.y);

    x_      = minX;
    y_      = minY;
    width_  = maxX - minX;
    height_ = maxY - minY;

    // Sync SFML view
    m_position = {x_, y_};
    m_size     = {width_, height_};
}
