//=============================================================================
// CanvasObject.cpp
//=============================================================================
#include "CanvasObject.h"
#include "VectorUtils.h"
#include <tuple>

CanvasObject::CanvasObject(const std::string& id, float x, float y, float w, float h, float rot)
    : id_(id), x_(x), y_(y), width_(w), height_(h), rotationDegrees_(rot),
      m_id(id), m_position(x, y), m_size(w, h), m_rotationDegrees_sfml(rot),
      m_isFlipped(false) {}

void CanvasObject::move(const sf::Vector2f& offset) {
    m_position += offset;
    x_ = m_position.x;
    y_ = m_position.y;
}

sf::Vector2f CanvasObject::getPosition() const { return m_position; }

void CanvasObject::move(float dx, float dy) {
    x_ += dx; 
    y_ += dy;
    m_position = {x_, y_};
}

void CanvasObject::setPosition(float x, float y) {
    x_ = x; 
    y_ = y;
    m_position = {x, y};
}

std::pair<float, float> CanvasObject::getPosition_floats() const { return {x_, y_}; }

void CanvasObject::setPosition(const sf::Vector2f& pos) {
    m_position = pos; 
    x_ = pos.x; 
    y_ = pos.y;
}

void CanvasObject::setSize(float w, float h) {
    width_ = w; 
    height_ = h;
    m_size = {w, h};
}

void CanvasObject::setSize(const sf::Vector2f& size) {
    m_size = size; 
    width_ = size.x; 
    height_ = size.y;
}

std::pair<float, float> CanvasObject::getSize_floats() const { return {width_, height_}; }

sf::Vector2f CanvasObject::getSize() const { return m_size; }

void CanvasObject::setRotation(float degrees) {
    rotationDegrees_ = degrees;
    m_rotationDegrees_sfml = degrees;
}

float CanvasObject::getRotation() const { return rotationDegrees_; }

// [NEW] Flip Implementation
void CanvasObject::setFlipped(bool flipped) {
    m_isFlipped = flipped;
}

bool CanvasObject::isFlipped() const {
    return m_isFlipped;
}

std::tuple<float, float, float, float> CanvasObject::getBoundingBox() const {
    return std::make_tuple(x_, y_, width_, height_);
}

const std::string& CanvasObject::getId() const { return id_; }