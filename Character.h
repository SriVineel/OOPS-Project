//=============================================================================
// Character.h
//=============================================================================
// PURPOSE:
//   In-scene character sprite that can be placed, moved, and resized.
//   Derives from CanvasObject for integration with canvas management system.
//
// KEY FEATURES:
//   - Position and size management via CanvasObject interface
//   - Flip support (flip X for left/right facing)
//   - Click detection for user interaction
//   - SFML sprite wrapping with texture reference
//
// WHERE TO MODIFY:
//   - Add animation: Store animation state and frame index
//   - Add properties: Extend with health, name, or other attributes
//   - Customize rendering: Modify draw() to add overlays or effects
//=============================================================================

#pragma once

#include "CanvasObject.h"
#include <string>
#include <SFML/Graphics.hpp> // sf::RenderWindow

class Character : public CanvasObject {
private:
    std::string imagePath_;        // Path to the image used for this character sprite.
    std::string expression_;       // Semantic expression ("neutral", "happy", etc.).
    bool flippedHorizontally_;     // If true, draw mirrored on X axis.

public:
    // Construct a character with an id, image path, and initial geometry.
    Character(const std::string& id,
              const std::string& imagePath,
              float x = 0.f, float y = 0.f,
              float width = 64.f, float height = 64.f);

    ~Character() override = default;

    // Draws the character; current implementation prints debug info (Phase 1 placeholder).
    void draw(sf::RenderWindow& window) override;

    // Simple AABB hit-test in logical coordinates.
    bool isClicked(float mouseX, float mouseY) const override;

    // Presentation state setters/getters
    void setExpression(const std::string& expr);
    const std::string& getExpression() const;

    void setImagePath(const std::string& path);
    const std::string& getImagePath() const;

    void setFlipped(bool flipped);
    bool isFlipped() const;
};
