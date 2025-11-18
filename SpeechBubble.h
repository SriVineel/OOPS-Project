//=============================================================================
// SpeechBubble.h
//=============================================================================
// PURPOSE:
//   Speech bubble component that displays text in various comic-style shapes.
//   Supports both procedurally-generated shapes and image-based bubbles.
//
// FEATURES:
//   - Multiple bubble styles: speech, thought, shout, rectangle
//   - Automatic text wrapping to fit bubble width
//   - Dynamic font sizing based on bubble dimensions
//   - Image-based or procedural rendering
//   - Centered text with style-specific adjustments
//
// BUBBLE STYLES:
//   "speech"           - Round bubble with tail (default)
//   "speech_rectangle" - Rectangular bubble with tail
//   "thought"          - Cloud-like thought bubble
//   "shout"            - Star-burst shout bubble
//
// WHERE TO MODIFY:
//   - Add new bubble styles: Create new rebuild*() method and add to setStyle()
//   - Change text layout: Modify centerText() and wrapText() methods
//   - Adjust font sizing: Modify setSize() font calculation
//   - Change tail position: Modify rebuild*() methods tail coordinates
//=============================================================================

#pragma once

#include <string>
#include <optional>
#include <SFML/Graphics.hpp>
#include "CanvasObject.h"

class SpeechBubble : public CanvasObject
{
public:
    // Constructor: Create bubble with text and geometry
    // Parameters:
    //   id: Unique identifier
    //   text: Display text (will be wrapped automatically)
    //   x, y: Position on canvas
    //   width, height: Bubble dimensions
    SpeechBubble(const std::string& id,
                 const std::string& text,
                 float x, float y,
                 float width, float height);

    //-------------------------------------------------------------------------
    // RENDERING - Modify SpeechBubble.cpp::draw() to change display
    //-------------------------------------------------------------------------

    // Draws bubble shape (or image) and centered text
    void draw(sf::RenderWindow& window) override;

    //-------------------------------------------------------------------------
    // HIT DETECTION
    //-------------------------------------------------------------------------

    // Simple AABB hit test (returns true if mouse is inside bubble bounds)
    bool isClicked(float mouseX, float mouseY) const override;

    //-------------------------------------------------------------------------
    // GEOMETRY OVERRIDES - Update text positioning when bubble moves/resizes
    //-------------------------------------------------------------------------

    // Override to keep text centered when position changes
    void setPosition(float x, float y) override;

    // Override to resize bubble and adjust font size dynamically
    void setSize(float w, float h) override;

    //-------------------------------------------------------------------------
    // TEXT PROPERTIES - Modify to change text appearance
    //-------------------------------------------------------------------------

    // Set/get bubble text content (triggers text wrapping)
    void setText(const std::string& text);
    std::string getText() const;

    // Set font size in pixels (triggers text rewrap)
    void setFontSize(int size);

    // Get current font size in pixels
    int getFontSize() const;

    // Set font by asset name (triggers text rewrap)
    void setFontName(const std::string& fname);

    //-------------------------------------------------------------------------
    // BUBBLE STYLE - Modify to change bubble appearance
    //-------------------------------------------------------------------------

    // Set bubble style: "speech", "thought", "shout", "speech_rectangle"
    // Will attempt to load image asset first, falls back to procedural
    void setStyle(const std::string& style);

private:
    //-------------------------------------------------------------------------
    // INTERNAL SHAPE BUILDERS - Modify to change bubble geometry
    //-------------------------------------------------------------------------

    // Master rebuild dispatcher (calls appropriate style method)
    void rebuild(float w, float h,
                 float radius = 12.f,
                 float tailLen = 20.f,
                 float tailWidth = 10.f);

    // Build round speech bubble with tail
    void rebuildSpeechRound(float w, float h,
                            float radius,
                            float tailLen,
                            float tailWidth);

    // Build rectangular speech bubble with tail
    void rebuildSpeechBox(float w, float h,
                          float radius,
                          float tailLen,
                          float tailWidth);

    // Build thought bubble (cloud-like shape with multiple blobs)
    void rebuildThought(float w, float h);

    // Build shout bubble (star-burst shape)
    void rebuildShout(float w, float h);

    //-------------------------------------------------------------------------
    // TEXT LAYOUT - Modify to change text positioning and wrapping
    //-------------------------------------------------------------------------

    // Center text within bubble (with style-specific offsets)
    void centerText();

    // Wrap text to fit 80% of bubble width
    void wrapText();

    //-------------------------------------------------------------------------
    // IMAGE LOADING - For image-based bubbles
    //-------------------------------------------------------------------------

    // Load and setup bubble background image
    void loadBubbleImage(const std::string& imagePath);

    //-------------------------------------------------------------------------
    // MEMBER VARIABLES
    //-------------------------------------------------------------------------

    sf::ConvexShape m_shape;              // Procedural bubble shape
    sf::Text m_text;                      // Text object for display
    std::string text_;                    // Current text content
    int fontSize_ = 24;                   // Current font size
    std::string fontName_ = "actionman";  // Current font asset name
    std::string style_ = "speech";        // Current bubble style
    bool useImageBubble_ = false;         // True if using image instead of shape
    std::string bubbleImagePath_;         // Asset key for bubble image
    std::optional<sf::Sprite> m_bubbleSprite; // Sprite for image-based bubble
};
