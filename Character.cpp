//=============================================================================
// Character.cpp
//=============================================================================
#include "Character.h"
#include "AssetManager.h"
#include <iostream> 

Character::Character(const std::string& id,
                     const std::string& imagePath,
                     float x, float y,
                     float width, float height)
    : CanvasObject(id, x, y, width, height, 0.f),
      imagePath_(imagePath),
      expression_("neutral")
{
}

void Character::draw(sf::RenderWindow& window)
{
    auto tex = AssetManager::getInstance().getTexture(imagePath_);
    if (!tex) return;

    sf::Sprite sprite(*tex);
    auto texSize = tex->getSize();
    if (texSize.x == 0 || texSize.y == 0) return;

    float sx = width_  / static_cast<float>(texSize.x);
    float sy = height_ / static_cast<float>(texSize.y);

    // [NEW] Handle flipping using base class state
    if (isFlipped())
    {
        sprite.setScale({-sx, sy});
        sprite.setOrigin({static_cast<float>(texSize.x), 0.f});
    }
    else
    {
        sprite.setScale({sx, sy});
        sprite.setOrigin({0.f, 0.f});
    }

    sprite.setPosition({x_, y_});
    sprite.setRotation(sf::degrees(rotationDegrees_));

    window.draw(sprite);
}

bool Character::isClicked(float mouseX, float mouseY) const
{
    bool inX = (mouseX >= x_ && mouseX <= x_ + width_);
    bool inY = (mouseY >= y_ && mouseY <= y_ + height_);
    return inX && inY;
}

void Character::setExpression(const std::string& expr) { expression_ = expr; }
const std::string& Character::getExpression() const { return expression_; }

void Character::setImagePath(const std::string& path) { imagePath_ = path; }
const std::string& Character::getImagePath() const { return imagePath_; }