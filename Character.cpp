// Character.cpp
// Phase 1 placeholder draw: logs where and how the character would be drawn.
// Future work: instantiate sf::Sprite with AssetManager texture and apply scale/flip.

#include "Character.h"
#include <iostream> // std::cout

Character::Character(const std::string& id, const std::string& imagePath,
                     float x, float y, float width, float height)
    : CanvasObject(id, x, y, width, height, 0.f),
      imagePath_(imagePath),
      expression_("neutral"),
      flippedHorizontally_(false) {}

void Character::draw(sf::RenderWindow&) {
    // Debug log instead of actual rendering in Phase 1
    std::cout << "[Draw] Character '" << id_ << "' at (" << x_ << "," << y_
              << "), size(" << width_ << "x" << height_ << "), expr=" << expression_
              << ", image=" << imagePath_
              << (flippedHorizontally_ ? " (flipped)" : "") << "\n";
}

bool Character::isClicked(float mouseX, float mouseY) const {
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
