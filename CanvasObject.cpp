//=============================================================================
// CanvasObject.cpp
//=============================================================================
// PURPOSE:
//   Implementation of CanvasObject base class methods.
//   Maintains synchronization between logical state and SFML rendering state.
//
// MODIFICATION GUIDE:
//   - All setter methods update BOTH logical (float) and SFML (Vector2f) values
//   - If you add new state variables, ensure they stay synchronized
//   - VectorUtils.h provides operator overloads for Vector2f math
//=============================================================================

#include "CanvasObject.h"
#include "VectorUtils.h"
#include <tuple>

// Constructor: Initialize both logical and SFML state with the same values
CanvasObject::CanvasObject(const std::string& id, float x, float y, float w, float h, float rot)
    : id_(id), x_(x), y_(y), width_(w), height_(h), rotationDegrees_(rot),
      m_id(id), m_position(x, y), m_size(w, h), m_rotationDegrees_sfml(rot) {}

// Move by offset vector (addition using VectorUtils operators)
void CanvasObject::move(const sf::Vector2f& offset) {
    m_position += offset;
    x_ = m_position.x;
    y_ = m_position.y;
}

// Get position as SFML vector
sf::Vector2f CanvasObject::getPosition() const { 
    return m_position; 
}

// Move by delta x and y (updates both representations)
void CanvasObject::move(float dx, float dy) {
    x_ += dx; 
    y_ += dy;
    m_position = {x_, y_};
}

// Set absolute position using floats
void CanvasObject::setPosition(float x, float y) {
    x_ = x; 
    y_ = y;
    m_position = {x, y};
}

// Get position as float pair (for UI display or serialization)
std::pair<float, float> CanvasObject::getPosition_floats() const { 
    return {x_, y_}; 
}

// Set position using SFML vector (keeps logical state in sync)
void CanvasObject::setPosition(const sf::Vector2f& pos) {
    m_position = pos; 
    x_ = pos.x; 
    y_ = pos.y;
}

// Set size using width and height
void CanvasObject::setSize(float w, float h) {
    width_ = w; 
    height_ = h;
    m_size = {w, h};
}

// Set size using SFML vector
void CanvasObject::setSize(const sf::Vector2f& size) {
    m_size = size; 
    width_ = size.x; 
    height_ = size.y;
}

// Get size as float pair
std::pair<float, float> CanvasObject::getSize_floats() const { 
    return {width_, height_}; 
}

// Get size as SFML vector
sf::Vector2f CanvasObject::getSize() const { 
    return m_size; 
}

// Set rotation in degrees (mirrors to SFML state)
void CanvasObject::setRotation(float degrees) {
    rotationDegrees_ = degrees;
    m_rotationDegrees_sfml = degrees;
}

// Get current rotation
float CanvasObject::getRotation() const { 
    return rotationDegrees_; 
}

// Get axis-aligned bounding box (useful for hit testing and selection)
std::tuple<float, float, float, float> CanvasObject::getBoundingBox() const {
    return std::make_tuple(x_, y_, width_, height_);
}

// Get unique identifier
const std::string& CanvasObject::getId() const { 
    return id_; 
}