//=============================================================================
// CanvasObject.h
//=============================================================================
// PURPOSE:
//   Base class for all drawable objects in the comic strip maker application.
//   Provides unified interface for position, size, rotation, and hit detection.
//
// USAGE:
//   Derive from this class to create custom canvas objects (characters, 
//   speech bubbles, brush strokes, etc.). Implement the pure virtual methods
//   draw() and isClicked().
//
// KEY CONCEPTS:
//   - Maintains both logical state (float values) and SFML state (Vector2f)
//   - All derived classes share consistent coordinate system and transformations
//   - Position (x_, y_): Top-left corner in logical coordinates
//   - Size (width_, height_): Object dimensions
//   - Rotation (rotationDegrees_): Clockwise rotation in degrees
//
// WHERE TO MODIFY:
//   - Add new transformation methods here if you need additional spatial operations
//   - Extend getBoundingBox() for more complex collision detection
//=============================================================================

#pragma once

#include <string>
#include <tuple>
#include <utility>
#include <SFML/Graphics.hpp>

class CanvasObject {
protected:
    // Logical state - used by application logic, persistence, and UI
    std::string id_;              // Unique identifier for this object
    float x_{0.f};                // X position (top-left corner)
    float y_{0.f};                // Y position (top-left corner)
    float width_{0.f};            // Object width
    float height_{0.f};           // Object height
    float rotationDegrees_{0.f};  // Rotation in degrees (clockwise)

    // SFML-side state - mirrors logical state for rendering
    std::string m_id;                    // Duplicated ID for SFML operations
    sf::Vector2f m_position{0.f, 0.f};   // SFML position vector
    sf::Vector2f m_size{0.f, 0.f};       // SFML size vector
    float m_rotationDegrees_sfml{0.f};   // SFML rotation value

public:
    // Constructor: Initialize with geometry and rotation
    // Parameters:
    //   id: Unique identifier string
    //   x, y: Initial position (top-left corner)
    //   width, height: Initial dimensions
    //   rotationDeg: Initial rotation in degrees
    CanvasObject(const std::string& id = "",
                 float x = 0.f, float y = 0.f,
                 float width = 0.f, float height = 0.f,
                 float rotationDeg = 0.f);

    virtual ~CanvasObject() = default;

    //-------------------------------------------------------------------------
    // PURE VIRTUAL METHODS - Must be implemented by derived classes
    //-------------------------------------------------------------------------

    // Render this object to the given window
    // Implementation should use SFML drawing primitives
    virtual void draw(sf::RenderWindow& window) = 0;

    // Hit test: return true if (mouseX, mouseY) intersects this object
    // Default implementation should use bounding box, override for precision
    virtual bool isClicked(float mouseX, float mouseY) const = 0;

    //-------------------------------------------------------------------------
    // POSITION METHODS - Modify where to find position-related operations
    //-------------------------------------------------------------------------

    // Move by offset vector (keeps logical and SFML states synchronized)
    void move(const sf::Vector2f& offset);

    // Move by delta x and delta y
    virtual void move(float dx, float dy);

    // Set absolute position using floats
    virtual void setPosition(float x, float y);

    // Set absolute position using SFML vector
    virtual void setPosition(const sf::Vector2f& pos);

    // Get current position as SFML vector
    sf::Vector2f getPosition() const;

    // Get current position as float pair
    virtual std::pair<float, float> getPosition_floats() const;

    //-------------------------------------------------------------------------
    // SIZE METHODS - Modify where to find size-related operations
    //-------------------------------------------------------------------------

    // Set size using width and height
    virtual void setSize(float w, float h);

    // Set size using SFML vector
    virtual void setSize(const sf::Vector2f& size);

    // Get size as SFML vector
    virtual sf::Vector2f getSize() const;

    // Get size as float pair
    virtual std::pair<float, float> getSize_floats() const;

    //-------------------------------------------------------------------------
    // ROTATION METHODS - Modify where to find rotation operations
    //-------------------------------------------------------------------------

    // Set rotation in degrees (clockwise)
    virtual void setRotation(float degrees);

    // Get current rotation in degrees
    virtual float getRotation() const;

    //-------------------------------------------------------------------------
    // UTILITY METHODS
    //-------------------------------------------------------------------------

    // Get axis-aligned bounding box as (x, y, width, height)
    // Useful for collision detection and selection
    virtual std::tuple<float, float, float, float> getBoundingBox() const;

    // Get unique identifier for this object
    const std::string& getId() const;
};