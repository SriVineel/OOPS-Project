//=============================================================================
// SpeechBubble.cpp
//=============================================================================
// PURPOSE:
//   Implements speech-bubble rendering, text layout and procedural shapes
//   used for dialogue, thought, and shout bubbles in the editor.
//
// NOTES:
//   - `wrapText` performs word-wrapping to the bubble width but intentionally
//     does not auto-shrink the font when the bubble is resized.
//   - Supports both procedural bubble shapes and optional image-based bubbles
//     loaded via the AssetManager.
//=============================================================================

#include "SpeechBubble.h"
#include "AssetManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
    constexpr float PI = 3.14159265358979323846f;
}

SpeechBubble::SpeechBubble(const std::string &id,
                           const std::string &text,
                           float x, float y,
                           float width, float height)
    : CanvasObject(id, x, y, width, height),
      m_text(AssetManager::getInstance().getFont("actionman"))
{
    text_ = text;
    fontName_ = "actionman";

    m_text.setString(text_);
    m_text.setCharacterSize(24u);
    fontSize_ = 24;
    m_text.setFillColor(sf::Color::Black);

    rebuild(width_, height_);
    setPosition(x, y);
    wrapText();
}

void SpeechBubble::wrapText()
{
    if (text_.empty())
    {
        m_text.setString("");
        centerText();
        return;
    }

    // Maximum width is 80% of bubble width
    float maxWidth = width_ * 0.80f;

    std::string wrappedText;
    std::string currentWord;
    std::string currentLine;

    for (char c : text_)
    {
        if (c == ' ' || c == '\n')
        {
            // Test if adding this word exceeds width
            std::string testLine = currentLine.empty() ? currentWord : currentLine + " " + currentWord;
            m_text.setString(testLine);
            auto bounds = m_text.getLocalBounds();

            if (bounds.size.x > maxWidth && !currentLine.empty())
            {
                // Line too long, wrap it
                wrappedText += currentLine + "\n";
                currentLine = currentWord;
            }
            else
            {
                currentLine = testLine;
            }

            currentWord.clear();

            if (c == '\n')
            {
                wrappedText += currentLine + "\n";
                currentLine.clear();
            }
        }
        else
        {
            // Add character to current word
            currentWord += c;

            // Check if the current word alone exceeds max width
            m_text.setString(currentWord);
            auto wordBounds = m_text.getLocalBounds();

            if (wordBounds.size.x > maxWidth)
            {
                // Word is too long, need to break it
                if (currentWord.length() > 1)
                {
                    // Move last character to next line
                    char lastChar = currentWord.back();
                    currentWord.pop_back();

                    // Add current word to wrapped text
                    if (!currentLine.empty())
                    {
                        wrappedText += currentLine + " ";
                    }
                    wrappedText += currentWord + "\n";

                    currentLine.clear();
                    currentWord = std::string(1, lastChar);
                }
            }
        }
    }

    // Handle last word
    if (!currentWord.empty())
    {
        std::string testLine = currentLine.empty() ? currentWord : currentLine + " " + currentWord;
        m_text.setString(testLine);
        auto bounds = m_text.getLocalBounds();

        if (bounds.size.x > maxWidth && !currentLine.empty())
        {
            wrappedText += currentLine + "\n" + currentWord;
        }
        else
        {
            wrappedText += testLine;
        }
    }
    else if (!currentLine.empty())
    {
        wrappedText += currentLine;
    }

    m_text.setString(wrappedText);

    // Note: we no longer auto-shrink the font when the bubble is resized.
    // Keep the current `fontSize_` and only reflow text to the new width.
    centerText();
}

void SpeechBubble::setSize(float w, float h)
{
    CanvasObject::setSize(w, h);

    // NOTE: Do not modify `fontSize_` here. The text size should remain
    // stable when the bubble is resized; only reflow/wrap the text to the
    // new dimensions.

    if (useImageBubble_ && m_bubbleSprite.has_value())
    {
        auto tex = AssetManager::getInstance().getTexture(bubbleImagePath_);
        if (tex)
        {
            auto texSize = tex->getSize();
            if (texSize.x > 0 && texSize.y > 0)
            {
                m_bubbleSprite->setScale(sf::Vector2f{
                    w / static_cast<float>(texSize.x),
                    h / static_cast<float>(texSize.y)});
            }
        }
    }
    else
    {
        rebuild(w, h);
    }

    wrapText(); // Re-wrap after size change
}

void SpeechBubble::centerText()
{
    auto bounds = m_text.getLocalBounds();

    // Set origin to the center of the text bounds
    m_text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                      bounds.position.y + bounds.size.y / 2.f});

    float centerX = m_position.x + width_ / 2.f;
    float centerY = m_position.y + height_ / 2.f;

    // Apply style-specific adjustments for vertical centering
    if (useImageBubble_)
    {
        if (style_ == "speech" || bubbleImagePath_ == "bubble_speech")
        {
            centerY -= height_ * 0.15f;
        }
        else if (style_ == "speech_round" || bubbleImagePath_ == "bubble_speech_round")
        {
            centerY -= height_ * 0.10f;
            centerX -= width_ * 0.05f;
        }
        else if (style_ == "thought" || bubbleImagePath_ == "bubble_thought")
        {
            centerY -= height_ * 0.08f;
        }
        else if (style_ == "speech_rectangle" || bubbleImagePath_ == "bubble_speech_rectangle")
        {
            centerY -= height_ * 0.12f;
        }
    }

    m_text.setPosition({centerX, centerY});
}

void SpeechBubble::rebuild(float w, float h,
                           float radius, float tailLen, float tailWidth)
{
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
    if (style_ == "speech_rectangle")
    {
        rebuildSpeechBox(w, h, radius, tailLen, tailWidth);
        return;
    }
    rebuildSpeechRound(w, h, radius, tailLen, tailWidth);
}

void SpeechBubble::rebuildSpeechRound(float w, float h,
                                      float radius,
                                      float tailLen,
                                      float tailWidth)
{
    const int arcSegments = 6;
    const int bodyPoints = 4 * arcSegments;
    m_shape.setPointCount(static_cast<std::size_t>(bodyPoints + 3));

    sf::Vector2f tl{radius, radius};
    sf::Vector2f tr{w - radius, radius};
    sf::Vector2f br{w - radius, h - radius};
    sf::Vector2f bl{radius, h - radius};

    auto putArc = [&](std::size_t startIdx,
                      const sf::Vector2f &c,
                      float startAng)
    {
        for (int i = 0; i < arcSegments; ++i)
        {
            float t = static_cast<float>(i) /
                      static_cast<float>(arcSegments - 1);
            float ang = startAng + t * (PI / 2.f);
            sf::Vector2f p{
                c.x + radius * std::cos(ang),
                c.y + radius * std::sin(ang)};
            m_shape.setPoint(startIdx + static_cast<std::size_t>(i), p);
        }
    };

    std::size_t idx = 0;
    putArc(idx, tl, PI);
    idx += arcSegments;
    putArc(idx, tr, -PI / 2.f);
    idx += arcSegments;
    putArc(idx, br, 0.f);
    idx += arcSegments;

    float baseX = radius + 18.f;
    float baseY = h;
    float halfW = tailWidth * 0.5f;
    m_shape.setPoint(idx++, {baseX + halfW, baseY - 2.f});
    m_shape.setPoint(idx++, {baseX - 0.20f * tailWidth, baseY + tailLen});
    m_shape.setPoint(idx++, {baseX - halfW, baseY - 2.f});

    putArc(idx, bl, PI / 2.f);

    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

void SpeechBubble::rebuildSpeechBox(float w, float h,
                                    float radius,
                                    float tailLen,
                                    float tailWidth)
{
    const int bodyPoints = 4;
    m_shape.setPointCount(static_cast<std::size_t>(bodyPoints + 3));

    std::size_t idx = 0;
    m_shape.setPoint(idx++, {radius, radius});
    m_shape.setPoint(idx++, {w - radius, radius});
    m_shape.setPoint(idx++, {w - radius, h - radius});

    float baseX = w - radius - 18.f;
    float baseY = h;
    float halfW = tailWidth * 0.5f;
    m_shape.setPoint(idx++, {baseX + halfW, baseY - 2.f});
    m_shape.setPoint(idx++, {baseX + 0.20f * tailWidth, baseY + tailLen});
    m_shape.setPoint(idx++, {baseX - halfW, baseY - 2.f});

    m_shape.setPoint(idx++, {radius, h - radius});

    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

void SpeechBubble::rebuildThought(float w, float h)
{
    const int blobs = 10;
    const int seg = 8;
    const float r = std::min(w, h) * 0.16f;
    m_shape.setPointCount(static_cast<std::size_t>(blobs * seg));

    const float a = (w * 0.5f) - r * 0.9f;
    const float b = (h * 0.5f) - r * 0.8f;
    const sf::Vector2f c{w * 0.5f, h * 0.5f};

    std::size_t idx = 0;
    for (int i = 0; i < blobs; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(blobs) * 2.f * PI;
        float jitter = 1.f + 0.15f * std::sin(t * 2.f);
        sf::Vector2f center{
            c.x + a * std::cos(t),
            c.y + b * std::sin(t)};

        for (int j = 0; j < seg; ++j)
        {
            float ang = static_cast<float>(j) / static_cast<float>(seg) * 2.f * PI;
            sf::Vector2f p{
                center.x + (r * jitter) * std::cos(ang),
                center.y + (r * jitter) * std::sin(ang)};
            m_shape.setPoint(idx++, p);
        }
    }

    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

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
        float t = static_cast<float>(i) /
                  static_cast<float>(spikes * 2) * 2.f * PI;
        float rad = (i % 2 == 0) ? rOut : rIn;
        sf::Vector2f p{
            c.x + rad * std::cos(t),
            c.y + rad * std::sin(t)};
        m_shape.setPoint(static_cast<std::size_t>(i), p);
    }

    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineColor(sf::Color::Black);
    m_shape.setOutlineThickness(2.f);
}

void SpeechBubble::loadBubbleImage(const std::string &imagePath)
{
    bubbleImagePath_ = imagePath;
    auto tex = AssetManager::getInstance().getTexture(imagePath);

    if (!tex)
    {
        std::cerr << "[SpeechBubble] Texture not found: " << imagePath << "\n";
        useImageBubble_ = false;
        rebuild(width_, height_);
        return;
    }

    try
    {
        m_bubbleSprite = sf::Sprite(*tex);
        useImageBubble_ = true;

        auto texSize = tex->getSize();
        if (texSize.x > 0 && texSize.y > 0)
        {
            m_bubbleSprite->setScale(sf::Vector2f{
                width_ / static_cast<float>(texSize.x),
                height_ / static_cast<float>(texSize.y)});
        }
        m_bubbleSprite->setPosition(m_position);
        std::cout << "[SpeechBubble] Loaded image: " << imagePath << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "[SpeechBubble] Exception: " << e.what() << "\n";
        useImageBubble_ = false;
        rebuild(width_, height_);
    }
}

void SpeechBubble::setStyle(const std::string &style)
{
    style_ = style;

    std::string imagePath = "bubble_" + style;
    auto tex = AssetManager::getInstance().getTexture(imagePath);

    if (tex)
    {
        std::cout << "[SpeechBubble] Found texture for: " << imagePath << "\n";
        loadBubbleImage(imagePath);
    }
    else
    {
        std::cerr << "[SpeechBubble] No texture for '" << imagePath
                  << "', using procedural\n";
        useImageBubble_ = false;
        rebuild(width_, height_);
    }
}

void SpeechBubble::draw(sf::RenderWindow &window)
{
    if (useImageBubble_ && m_bubbleSprite.has_value())
    {
        window.draw(*m_bubbleSprite);
    }
    else
    {
        window.draw(m_shape);
    }

    window.draw(m_text);
}

bool SpeechBubble::isClicked(float mouseX, float mouseY) const
{
    bool inX = (mouseX >= x_ && mouseX <= x_ + width_);
    bool inY = (mouseY >= y_ && mouseY <= y_ + height_);
    return inX && inY;
}

void SpeechBubble::setPosition(float x, float y)
{
    CanvasObject::setPosition(x, y);
    m_shape.setPosition(m_position);

    if (m_bubbleSprite.has_value())
    {
        m_bubbleSprite->setPosition(m_position);
    }
    centerText();
}

void SpeechBubble::setText(const std::string &text)
{
    text_ = text;
    wrapText();
}

std::string SpeechBubble::getText() const
{
    return text_;
}

void SpeechBubble::setFontSize(int size)
{
    fontSize_ = size;
    m_text.setCharacterSize(static_cast<unsigned int>(size));
    wrapText();
}

int SpeechBubble::getFontSize() const
{
    return fontSize_;
}

void SpeechBubble::setFontName(const std::string &fname)
{
    fontName_ = fname;
    try
    {
        m_text.setFont(AssetManager::getInstance().getFont(fname));
        wrapText();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[SpeechBubble] Font error: " << e.what() << "\n";
    }
}
