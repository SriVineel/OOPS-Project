// SpeechBubble.cpp

// Implements polygon generation for "speech", "thought", and "shout" styles.
// Design notes:
// - One sf::ConvexShape is used per bubble to compose both the body and tail.
// - setPosition and setText keep the m_text baseline/padding consistent after rebuilds.
// - SFML 3 changes: Text bounds use bounds.position as origin (not left/top).

#include "SpeechBubble.h"
#include "AssetManager.h"
#include <algorithm> // std::min
#include <cmath>     // std::cos, std::sin

namespace
{
    constexpr float PI = 3.14159265358979323846f;
}

SpeechBubble::SpeechBubble(const std::string &id, const std::string &text,
                           float x, float y, float width, float height)
    : CanvasObject(id, x, y, width, height),
      m_font(AssetManager::getInstance().getFont("main_font")),
      m_text(m_font, text, 14u)
{
    text_ = text;
    m_text.setFillColor(sf::Color::Black);
    rebuild(width_, height_);
    setPosition(x, y);
}

// MODIFIED: Dynamic font sizing with larger scaling (divisor = 3.5 instead of 4.5)
void SpeechBubble::setSize(float w, float h)
{
    CanvasObject::setSize(w, h);

    // Auto-calculate font size based on bubble dimensions
    float minDim = std::min(w, h);
    int autoSize = static_cast<int>(minDim / 3.5f); // Larger font scaling

    // Clamp between 12 and 60 pixels
    autoSize = std::max(20, std::min(autoSize, 60));

    setFontSize(autoSize);
    rebuild(w, h);
}

// NEW: Helper function to center text in the bubble
void SpeechBubble::centerText()
{
    auto bounds = m_text.getLocalBounds();

    m_text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                      bounds.position.y + bounds.size.y / 2.f});

    // Position at center of bubble
    m_text.setPosition({m_position.x + width_ / 2.f,
                        m_position.y + height_ / 2.f});
}

void SpeechBubble::rebuild(float w, float h, float r, float tailLen, float tailW)
{
    // Dispatch to style-specific builder
    if (style_ == "thought")
    {
        rebuildThought(w, h);
        return;
    }
    if (style_ == "shout")
    {
        rebuildShout(w, h);
        return;
    }
    rebuildSpeech(w, h, r, tailLen, tailW); // default "speech"
}

// Rounded rectangle + triangular tail (merged polygon)
void SpeechBubble::rebuildSpeech(float w, float h, float radius, float tailLen, float tailW)
{
    const int arcSegments = 6;                                       // points per corner arc for smoothness
    const int bodyPoints = 4 * arcSegments;                          // 4 arcs => 4 corners
    m_shape.setPointCount(static_cast<std::size_t>(bodyPoints + 3)); // +3 for the tail

    // Corner centers (in local coordinates; shape's position is applied at draw time)
    sf::Vector2f tl{radius, radius};
    sf::Vector2f tr{w - radius, radius};
    sf::Vector2f br{w - radius, h - radius};
    sf::Vector2f bl{radius, h - radius};

    // Helper to emit an arc quarter
    auto putArc = [&](std::size_t startIdx, const sf::Vector2f &c, float startAng)
    {
        for (int i = 0; i < arcSegments; ++i)
        {
            float t = static_cast<float>(i) / (arcSegments - 1);
            float ang = startAng + t * (PI / 2.f);
            sf::Vector2f p{c.x + radius * std::cos(ang), c.y + radius * std::sin(ang)};
            m_shape.setPoint(startIdx + static_cast<std::size_t>(i), p);
        }
    };
    std::size_t idx = 0;
    putArc(idx, tl, PI);
    idx += arcSegments; // top-left
    putArc(idx, tr, -PI / 2.f);
    idx += arcSegments; // top-right
    putArc(idx, br, 0.f);
    idx += arcSegments; // bottom-right

    // Tail along bottom edge near left (tweak baseX to move tail sideways)
    float baseX = radius + 18.f;
    float baseY = h;
    float halfW = tailW * 0.5f;
    m_shape.setPoint(idx++, {baseX + halfW, baseY - 2.f});             // base right
    m_shape.setPoint(idx++, {baseX - 0.20f * tailW, baseY + tailLen}); // tip (slightly left for tilt)
    m_shape.setPoint(idx++, {baseX - halfW, baseY - 2.f});             // base left
    putArc(idx, bl, PI / 2.f);                                         // bottom-left to top-left

    // Style
    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

// Thought bubble: filled cloud with many small circles approximated by convex polygon
void SpeechBubble::rebuildThought(float w, float h)
{
    const int blobs = 18; // higher => smoother perimeter
    const int seg = 10;   // points per blob
    const float r = std::min(w, h) * 0.14f;
    m_shape.setPointCount(static_cast<std::size_t>(blobs * seg));

    // Elliptical distribution of blob centers
    const float a = (w * 0.5f) - r - 2.f;
    const float b = (h * 0.5f) - r - 2.f;
    const sf::Vector2f c{w * 0.5f, h * 0.5f};
    std::size_t idx = 0;
    for (int i = 0; i < blobs; ++i)
    {
        float t = static_cast<float>(i) / blobs * 2.f * PI;
        sf::Vector2f center{c.x + a * std::cos(t), c.y + b * std::sin(t)};
        for (int j = 0; j < seg; ++j)
        {
            float ang = static_cast<float>(j) / seg * 2.f * PI;
            sf::Vector2f p{center.x + r * std::cos(ang), center.y + r * std::sin(ang)};
            m_shape.setPoint(idx++, p);
        }
    }
    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

// Shout bubble: spiky star-like polygon with alternating radii
void SpeechBubble::rebuildShout(float w, float h)
{
    const int spikes = 16;
    const float rx = w * 0.48f;
    const float ry = h * 0.42f;
    const float rIn = std::min(rx, ry) * 0.65f;
    const float rOut = std::min(rx, ry);
    m_shape.setPointCount(static_cast<std::size_t>(spikes * 2));
    const sf::Vector2f c{w * 0.5f, h * 0.5f};
    for (int i = 0; i < spikes * 2; ++i)
    {
        float t = static_cast<float>(i) / (spikes * 2) * 2.f * PI;
        float rad = (i % 2 == 0) ? rOut : rIn;
        sf::Vector2f p{c.x + rad * std::cos(t), c.y + rad * std::sin(t)};
        m_shape.setPoint(static_cast<std::size_t>(i), p);
    }
    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

void SpeechBubble::draw(sf::RenderWindow &window)
{
    window.draw(m_shape);
    window.draw(m_text);
}

bool SpeechBubble::isClicked(float mouseX, float mouseY) const
{
    // AABB hit test over logical bounds; adequate because shape is convex/contained.
    bool inX = (mouseX >= x_ && mouseX <= x_ + width_);
    bool inY = (mouseY >= y_ && mouseY <= y_ + height_);
    return inX && inY;
}

// MODIFIED: Now uses centerText() helper for center alignment
void SpeechBubble::setPosition(float x, float y)
{
    CanvasObject::setPosition(x, y);
    m_shape.setPosition(m_position);
    centerText();
}

void SpeechBubble::setPosition(const sf::Vector2f &pos) { setPosition(pos.x, pos.y); }

// MODIFIED: Now uses centerText() helper for center alignment
void SpeechBubble::setText(const std::string &text)
{
    text_ = text;
    m_text.setString(text_);
    centerText();
}

std::string SpeechBubble::getText() const { return text_; }

// MODIFIED: Now uses centerText() helper for center alignment
void SpeechBubble::setFontSize(int size)
{
    fontSize_ = size;
    m_text.setCharacterSize(static_cast<unsigned int>(size));
    centerText();
}
