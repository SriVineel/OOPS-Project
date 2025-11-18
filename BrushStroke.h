//=============================================================================
// BrushStroke.h
//=============================================================================
// PURPOSE:
//   Free-hand brush stroke drawn by the user.
//   Stores points as circles connected in sequence for smooth appearance.
//
// KEY FEATURES:
//   - Point interpolation: Fills gaps for smooth lines at high speeds
//   - Variable thickness: Circles drawn at each point to simulate brush width
//   - Color management: Each stroke stores and can change its color
//   - Bounds tracking: Maintains logical bounding box for selection
//
// USAGE:
//   1. Create stroke with color and thickness
//   2. Call beginAt(startPosition) to initialize
//   3. Call addPoint(position) for each mouse position during drag
//   4. Call draw(window) to render
//
// WHERE TO MODIFY:
//   - Change rendering: Switch from circles to line segments
//   - Add texture: Apply pattern or gradient to strokes
//   - Add effects: Implement pressure sensitivity or fade-out
//=============================================================================

#pragma once

#include "CanvasObject.h"

#include <SFML/Graphics.hpp>
#include <string>

class BrushStroke : public CanvasObject {
public:
    explicit BrushStroke(const std::string& id,
                         const sf::Color& color = sf::Color::Black,
                         float thickness = 1.f);

    // Start a new stroke at the given world position
    void beginAt(const sf::Vector2f& pos);

    // Append a new point to the stroke (world position)
    void addPoint(const sf::Vector2f& pos);

    void setColor(const sf::Color& c);
    sf::Color getColor() const;

    // CanvasObject interface
    void draw(sf::RenderWindow& window) override;
    bool isClicked(float mouseX, float mouseY) const override;

private:
    sf::VertexArray m_vertices;  // Line strip representing the stroke
    sf::Color color_;
    float thickness_;            // Reserved for future use (fixed 1px for now)

    // Keep CanvasObject's logical bounds (x_, y_, width_, height_) in sync
    void updateBoundsForPoint(const sf::Vector2f& p);
};
