//=============================================================================
// Character.cpp
//=============================================================================
// PURPOSE:
//   Concrete CanvasObject representing a character. Renders a texture loaded
//   through the AssetManager and provides basic properties such as
//   expression and horizontal flipping.
//
// NOTES:
//   - Scales and positions the sprite to match logical dimensions.
//   - Uses AABB hit testing; refine if pixel-accurate hit tests are desired.
//=============================================================================

#include "Character.h"
#include "AssetManager.h"

#include <iostream> // std::cout

Character::Character(const std::string& id,
                     const std::string& imagePath,
                     float x, float y,
                     float width, float height)
    : CanvasObject(id, x, y, width, height, 0.f),
      imagePath_(imagePath),
      expression_("neutral"),
      flippedHorizontally_(false)
{
}

void Character::draw(sf::RenderWindow& window)
{
    // Look up texture by logical name (imagePath_ is the key)
    auto tex = AssetManager::getInstance().getTexture(imagePath_);
    if (!tex)
    {
        std::cout << "[Draw] Character '" << id_
                  << "' missing texture for key '" << imagePath_ << "'\n";
        return;
    }

    sf::Sprite sprite(*tex);

    // Sanity check
    auto texSize = tex->getSize();
    if (texSize.x == 0 || texSize.y == 0)
    {
        std::cout << "[Draw] Character '" << id_
                  << "' has zero-size texture '" << imagePath_ << "'\n";
        return;
    }

    // Scale sprite to match logical width_/height_
    float sx = width_  / static_cast<float>(texSize.x);
    float sy = height_ / static_cast<float>(texSize.y);

    // SFML 3: setScale / setOrigin use Vector2f
    if (flippedHorizontally_)
    {
        sprite.setScale({-sx, sy});
        sprite.setOrigin({static_cast<float>(texSize.x), 0.f});
    }
    else
    {
        sprite.setScale({sx, sy});
        sprite.setOrigin({0.f, 0.f});
    }

    // SFML 3: setPosition uses Vector2f
    sprite.setPosition({x_, y_});

    // SFML 3: rotation uses sf::Angle
    sprite.setRotation(sf::degrees(rotationDegrees_));

    window.draw(sprite);
}

bool Character::isClicked(float mouseX, float mouseY) const
{
    // AABB hit test using logical coordinates
    bool inX = (mouseX >= x_ && mouseX <= x_ + width_);
    bool inY = (mouseY >= y_ && mouseY <= y_ + height_);
    return inX && inY;
}

void Character::setExpression(const std::string& expr) { expression_ = expr; }
const std::string& Character::getExpression() const { return expression_; }

void Character::setImagePath(const std::string& path) { imagePath_ = path; }
const std::string& Character::getImagePath() const { return imagePath_; }

void Character::setFlipped(bool flipped) { flippedHorizontally_ = flipped; }
bool Character::isFlipped() const { return flippedHorizontally_; }
