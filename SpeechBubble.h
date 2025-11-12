// SpeechBubble.h

// Purpose: A text bubble that can render as "speech", "thought" (cloud), or "shout" (spiky).
// Responsibilities:
// - Keep logical text/font/style state.
// - Own the SFML primitives (sf::Text, sf::ConvexShape) necessary to render the chosen style.
// - Rebuild polygonal geometry when size/style changes.

#pragma once

#include "CanvasObject.h"
#include <SFML/Graphics.hpp>
#include <string>

class SpeechBubble : public CanvasObject {
private:
    // Logical state
    std::string text_;              // Plain text content
    std::string fontName_;          // Logical font name (from AssetManager), if needed later
    int fontSize_{14};              // Pixel character size
    std::string style_{"speech"};   // "speech" | "thought" | "shout"

    // Encapsulated drawing state
    sf::Font& m_font;               // Non-owning reference to font from AssetManager
    sf::Text m_text;                // Text object positioned relative to bubble
    sf::ConvexShape m_shape;        // Merged polygon (body + tail) to minimize draw calls

    // Rebuild dispatch + helpers
    void rebuild(float w, float h, float radius = 12.f,
                 float tailLen = 18.f, float tailWidth = 14.f);
    void rebuildSpeech(float w, float h, float r, float tailLen, float tailW);
    void rebuildThought(float w, float h);  // "cloud" style
    void rebuildShout(float w, float h);    // "spiky" style
    
    // NEW: Helper to center text in the bubble
    void centerText();

public:
    // Construct a bubble with default style and geometry.
    SpeechBubble(const std::string& id,
                 const std::string& text = "",
                 float x = 0.f, float y = 0.f,
                 float width = 150.f, float height = 60.f);
    ~SpeechBubble() override = default;

    void draw(sf::RenderWindow& window) override;
    bool isClicked(float mouseX, float mouseY) const override;

    // Keep both logical and SFML geometry aligned on position change.
    void setPosition(float x, float y) override;
    void setPosition(const sf::Vector2f& pos) override;

    // Size change triggers shape rebuild for visual consistency.
    // MODIFIED: Changed from inline to regular method for dynamic font sizing
    void setSize(float w, float h) override;
    void setSize(const sf::Vector2f& s) override { setSize(s.x, s.y); }

    // Text and typography controls
    void setText(const std::string& text);
    std::string getText() const;
    void setFontName(const std::string& fname) { fontName_ = fname; }
    std::string getFontName() const { return fontName_; }
    void setFontSize(int size);
    int getFontSize() const { return fontSize_; }

    // Changing style triggers a rebuild to update the polygon.
    void setStyle(const std::string& style) { style_ = style; rebuild(width_, height_); }
    std::string getStyle() const { return style_; }
};
