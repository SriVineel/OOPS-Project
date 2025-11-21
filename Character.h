//=============================================================================
// Character.h
//=============================================================================
#pragma once

#include "CanvasObject.h"
#include <string>
#include <SFML/Graphics.hpp> 

class Character : public CanvasObject {
private:
    std::string imagePath_;        
    std::string expression_;       

public:
    Character(const std::string& id,
              const std::string& imagePath,
              float x = 0.f, float y = 0.f,
              float width = 64.f, float height = 64.f);

    ~Character() override = default;

    void draw(sf::RenderWindow& window) override;
    bool isClicked(float mouseX, float mouseY) const override;

    void setExpression(const std::string& expr);
    const std::string& getExpression() const;

    void setImagePath(const std::string& path);
    const std::string& getImagePath() const;
};