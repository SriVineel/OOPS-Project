// CanvasObject.h
// Purpose: Base class for anything placed on the canvas (characters, bubbles, etc.).
// Key idea: Maintain a "logical state" (id_, x_, y_, width_, height_, rotationDegrees_) alongside
// an "SFML view" (m_position, m_size, m_rotationDegrees_sfml) so higher-level logic can work with
// simple floats while render code uses SFML-native types.
// Derive from this class and implement draw(...) and isClicked(...).

#pragma once

#include <string>             // std::string
#include <tuple>              // std::tuple
#include <utility>            // std::pair
#include <SFML/Graphics.hpp>  // sf::RenderWindow, sf::Vector2f

class CanvasObject {
protected:
    // Logical state (used by app logic, persistence, etc.)
    std::string id_;
    float x_{0.f};
    float y_{0.f};
    float width_{0.f};
    float height_{0.f};
    float rotationDegrees_{0.f};

    // Encapsulated SFML view mirroring the logical state (used by drawing)
    std::string m_id;                 // duplicated for clarity in SFML-side operations
    sf::Vector2f m_position{0.f, 0.f};
    sf::Vector2f m_size{0.f, 0.f};
    float m_rotationDegrees_sfml{0.f};

public:
    // Construct with initial geometry and rotation; rotation is in degrees.
    CanvasObject(const std::string& id = "",
                 float x = 0.f, float y = 0.f,
                 float width = 0.f, float height = 0.f,
                 float rotationDeg = 0.f);

    virtual ~CanvasObject() = default;

    // Must draw itself to the given window.
    virtual void draw(sf::RenderWindow& window) = 0;

    // Returns true if the (mouseX, mouseY) is inside the object's logical bounds.
    virtual bool isClicked(float mouseX, float mouseY) const = 0;

    // Movement helpers keep both logical and SFML state in sync.
    void move(const sf::Vector2f& offset);
    sf::Vector2f getPosition() const;
    virtual void move(float dx, float dy);
    virtual void setPosition(float x, float y);
    virtual std::pair<float,float> getPosition_floats() const;
    virtual void setPosition(const sf::Vector2f& pos);

    // Size helpers
    virtual void setSize(float w, float h);
    virtual void setSize(const sf::Vector2f& size);
    virtual std::pair<float,float> getSize_floats() const;
    virtual sf::Vector2f getSize() const;

    // Rotation helpers (degrees)
    virtual void setRotation(float degrees);
    virtual float getRotation() const;

    // Axis-aligned bounding box in logical coordinates.
    virtual std::tuple<float,float,float,float> getBoundingBox() const;

    // Stable identifier useful for selection and persistence.
    const std::string& getId() const;
};
