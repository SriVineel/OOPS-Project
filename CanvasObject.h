//=============================================================================
// CanvasObject.h
//=============================================================================
#pragma once

#include <string>
#include <tuple>
#include <utility>
#include <SFML/Graphics.hpp>

class CanvasObject {
protected:
    // Logical state
    std::string id_;              
    float x_{0.f};                
    float y_{0.f};                
    float width_{0.f};            
    float height_{0.f};           
    float rotationDegrees_{0.f};  

    // SFML-side state
    std::string m_id;                    
    sf::Vector2f m_position{0.f, 0.f};   
    sf::Vector2f m_size{0.f, 0.f};       
    float m_rotationDegrees_sfml{0.f};   
    bool m_isFlipped{false};             // [NEW] Horizontal flip state

public:
    CanvasObject(const std::string& id = "",
                 float x = 0.f, float y = 0.f,
                 float width = 0.f, float height = 0.f,
                 float rotationDeg = 0.f);

    virtual ~CanvasObject() = default;

    // Pure virtual methods
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual bool isClicked(float mouseX, float mouseY) const = 0;

    // Position
    void move(const sf::Vector2f& offset);
    virtual void move(float dx, float dy);
    virtual void setPosition(float x, float y);
    virtual void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    virtual std::pair<float, float> getPosition_floats() const;

    // Size
    virtual void setSize(float w, float h);
    virtual void setSize(const sf::Vector2f& size);
    virtual sf::Vector2f getSize() const;
    virtual std::pair<float, float> getSize_floats() const;

    // Rotation
    virtual void setRotation(float degrees);
    virtual float getRotation() const;

    // [NEW] Transform Methods
    virtual void setFlipped(bool flipped);
    virtual bool isFlipped() const;

    // Utility
    virtual std::tuple<float, float, float, float> getBoundingBox() const;
    const std::string& getId() const;
};