//=============================================================================
// main.cpp
//=============================================================================
// PURPOSE:
//   Main application entry point for Comic Strip Maker.
//   Manages the application loop, UI rendering, event handling, and scene management.
//
// KEY FEATURES:
//   - [UPDATED] Flip Handle: Click the Cyan square on a selected object to flip it.
//   - [UPDATED] Export Button: Restored to sidebar position above sliders.
//   - Auto-discovery asset loading
//   - Draw mode & Eraser tools
//   - Undo/Redo system
//   - Interactive palette
//=============================================================================

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <filesystem>

#include "AssetManager.h"
#include "SpeechBubble.h"
#include "VectorUtils.h"
#include "Character.h"
#include "BrushStroke.h"
#include "Command.h"

// ----------------------------------------------------------------------------
// Enums and Structures
// ----------------------------------------------------------------------------

enum class Category
{
    Characters,
    Fonts,
    Bubbles
};

struct PaletteItem
{
    std::string assetKey;
    std::string assetType;
    sf::FloatRect hit;
};

struct CategoryHeader
{
    Category category;
    sf::FloatRect hit;
};

enum class PickKind
{
    None,
    Sprite,
    Bubble
};

// ----------------------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------------------

// Simple HSV -> RGB helper for the color wheel (h,s,v in [0,1])
static sf::Color hsvToRgb(float h, float s, float v)
{
    float r = 0.f, g = 0.f, b = 0.f;
    float i = std::floor(h * 6.f);
    float f = h * 6.f - i;
    float p = v * (1.f - s);
    float q = v * (1.f - f * s);
    float t = v * (1.f - (1.f - f) * s);
    int ii = static_cast<int>(i) % 6;

    switch (ii)
    {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
    }

    return sf::Color(
        static_cast<std::uint8_t>(r * 255.f),
        static_cast<std::uint8_t>(g * 255.f),
        static_cast<std::uint8_t>(b * 255.f));
}

// ----------------------------------------------------------------------------
// Main Application Entry
// ----------------------------------------------------------------------------

int main()
{
    // 1) AUTO-LOAD ASSETS
    try
    {
        auto &AM = AssetManager::getInstance();

        std::cout << "====================================\n";
        std::cout << " Comic Strip Maker - Asset Loader\n";
        std::cout << "====================================\n\n";

        AM.autoLoadCharacters("assets/Characters");
        AM.autoLoadFonts("assets/Font");
        AM.autoLoadBubbles("assets/SpeechBubbles");

        std::cout << "\n[Main] All assets loaded successfully!\n";
        std::cout << "====================================\n\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERROR] Failed to load assets: " << e.what() << "\n";
        return 1;
    }

    // 2) Dynamic Window Setup
    auto desktop = sf::VideoMode::getDesktopMode();
    unsigned int screenWidth = desktop.size.x;
    unsigned int screenHeight = desktop.size.y;

    unsigned int windowWidth = static_cast<unsigned int>(screenWidth * 0.95f);
    unsigned int windowHeight = static_cast<unsigned int>(screenHeight * 0.95f);

    sf::RenderWindow window(
        sf::VideoMode({windowWidth, windowHeight}),
        sf::String("Comic Strip Maker - Final"));
    window.setFramerateLimit(60);

    int windowX = static_cast<int>((screenWidth - windowWidth) / 2);
    int windowY = static_cast<int>((screenHeight - windowHeight) / 2);
    window.setPosition({windowX, windowY});

    const float SidebarW = 200.f;

    sf::RectangleShape sidebarBg(sf::Vector2f{SidebarW, static_cast<float>(windowHeight)});
    sidebarBg.setFillColor(sf::Color::White);
    sidebarBg.setOutlineColor(sf::Color(200, 200, 200));
    sidebarBg.setOutlineThickness(1.f);

    // 3) Category headers
    const float headerH = 32.f;
    const float headerPad = 8.f;

    std::vector<CategoryHeader> headers = {
        {Category::Characters, sf::FloatRect{{0.f, 0.f}, {SidebarW, headerH}}},
        {Category::Fonts, sf::FloatRect{{0.f, headerH}, {SidebarW, headerH}}},
        {Category::Bubbles, sf::FloatRect{{0.f, 2.f * headerH}, {SidebarW, headerH}}}};
    Category currentCategory = Category::Characters;

    // 4) UI Buttons Initialization
    auto &AM = AssetManager::getInstance();

    // Shared dimensions for split buttons
    float buttonWidth = (SidebarW - 30.f) / 2.f;

    // --- Draw Mode Button (Left Bottom) ---
    sf::RectangleShape drawButton(sf::Vector2f{buttonWidth, 40.f});
    drawButton.setFillColor(sf::Color(200, 200, 200));
    drawButton.setOutlineColor(sf::Color(150, 150, 150));
    drawButton.setOutlineThickness(2.f);

    sf::Text drawButtonText(AM.getFont("actionman"));
    drawButtonText.setString("Draw");
    drawButtonText.setCharacterSize(16);
    drawButtonText.setFillColor(sf::Color::Black);

    // --- Eraser Button (Right Bottom) ---
    sf::RectangleShape eraserButton(sf::Vector2f{buttonWidth, 40.f});
    eraserButton.setFillColor(sf::Color(200, 200, 200));
    eraserButton.setOutlineColor(sf::Color(150, 150, 150));
    eraserButton.setOutlineThickness(2.f);

    sf::Text eraserButtonText(AM.getFont("actionman"));
    eraserButtonText.setString("Eraser");
    eraserButtonText.setCharacterSize(16);
    eraserButtonText.setFillColor(sf::Color::Black);

    // --- Undo/Redo Buttons (Above Draw/Eraser) ---
    sf::RectangleShape undoButton(sf::Vector2f{buttonWidth, 40.f});
    undoButton.setFillColor(sf::Color(200, 200, 200));
    undoButton.setOutlineColor(sf::Color(150, 150, 150));
    undoButton.setOutlineThickness(2.f);

    sf::Text undoButtonText(AM.getFont("actionman"));
    undoButtonText.setString("Undo");
    undoButtonText.setCharacterSize(16);
    undoButtonText.setFillColor(sf::Color::Black);

    sf::RectangleShape redoButton(sf::Vector2f{buttonWidth, 40.f});
    redoButton.setFillColor(sf::Color(200, 200, 200));
    redoButton.setOutlineColor(sf::Color(150, 150, 150));
    redoButton.setOutlineThickness(2.f);

    sf::Text redoButtonText(AM.getFont("actionman"));
    redoButtonText.setString("Redo");
    redoButtonText.setCharacterSize(16);
    redoButtonText.setFillColor(sf::Color::Black);

    // --- Export Button (Above Thickness) ---
    sf::RectangleShape exportButton(sf::Vector2f{SidebarW - 20.f, 40.f});
    exportButton.setFillColor(sf::Color(200, 200, 200));
    exportButton.setOutlineColor(sf::Color(150, 150, 150));
    exportButton.setOutlineThickness(2.f);

    sf::Text exportButtonText(AM.getFont("actionman"));
    exportButtonText.setString("Export Image");
    exportButtonText.setCharacterSize(16);
    exportButtonText.setFillColor(sf::Color::Black);

    // State Flags
    bool saveNextFrame = false;
    bool isExportHovered = false;
    bool isEraserHovered = false;
    bool eraserActive = false;

    // 5) Command Manager
    CommandManager commandManager;

    // 6) Scene containers
    std::vector<std::unique_ptr<Character>> characters;
    std::vector<std::unique_ptr<SpeechBubble>> bubbles;
    std::vector<std::unique_ptr<BrushStroke>> strokes;

    // 7) Palette Data
    std::vector<PaletteItem> palette;

    auto rebuildPalette = [&]()
    {
        palette.clear();

        float startY = headers.back().hit.position.y + headers.back().hit.size.y + headerPad;
        float rowH = 60.f;
        float x = 16.f;
        float w = SidebarW - 32.f;

        auto addRow = [&](const std::string &key, const std::string &type)
        {
            palette.push_back(PaletteItem{key, type, sf::FloatRect{{x, startY}, {w, rowH}}});
            startY += rowH + headerPad;
        };

        switch (currentCategory)
        {
        case Category::Characters:
            for (const auto &key : AM.getAssetsByType("CHARACTER"))
            {
                addRow(key, "CHARACTER");
            }
            break;
        case Category::Fonts:
            for (const auto &key : AM.getAssetsByType("FONT"))
            {
                addRow(key, "FONT");
            }
            break;
        case Category::Bubbles:
            for (const auto &key : AM.getAssetsByType("BUBBLE"))
            {
                addRow(key, "BUBBLE");
            }
            break;
        }
    };

    rebuildPalette();

    // 8) Interaction state variables
    bool draggingSprite = false;
    int dragSpriteIdx = -1;
    bool draggingBubble = false;
    int dragBubbleIdx = -1;
    sf::Vector2f dragOffset{0.f, 0.f};

    SpeechBubble *activeBubble = nullptr;

    PickKind picked = PickKind::None;
    int pickedIndex = -1;

    bool resizing = false;
    PickKind resizeKind = PickKind::None;
    int resizeIndex = -1;
    sf::Vector2f resizeStartMouse{0.f, 0.f};
    sf::Vector2f resizeStartSize{0.f, 0.f};
    sf::Vector2f resizeStartPos{0.f, 0.f};

    bool drawMode = false;
    BrushStroke *activeStroke = nullptr;

    // Brush configuration state
    sf::Color currentBrushColor = sf::Color::Black;
    float currentBrushThickness = 4.f;
    const float minBrushThickness = 1.f;
    const float maxBrushThickness = 20.f;
    bool draggingThickness = false;
    bool pickingColor = false;

    // Text size input state (numeric entry)
    float currentTextSize = 24.f;
    const float minTextSize = 8.f;
    const float maxTextSize = 72.f;
    bool draggingTextSize = false;
    sf::FloatRect fontSectionBounds;

    // Text size slider (only visible in font section)
    sf::RectangleShape textSizeBar(sf::Vector2f(SidebarW - 40.f, 4.f));
    textSizeBar.setFillColor(sf::Color(100, 100, 100));
    textSizeBar.setPosition(sf::Vector2f(20.f, 200.f)); // Default position

    sf::CircleShape textSizeHandle(8.f);
    textSizeHandle.setFillColor(sf::Color(60, 60, 60));
    // Use center-origin so scaling/positioning is easier
    textSizeHandle.setOrigin(sf::Vector2f(textSizeHandle.getRadius(), textSizeHandle.getRadius()));

    // Animation / hover state for slider handle
    int textSizeOld = -1;
    float handleScale = 1.f;
    bool hoverTextSizeSlider = false;
    sf::Clock hoverClock;

    auto updateTextSizeHandle = [&]()
    {
        float t = (currentTextSize - minTextSize) / (maxTextSize - minTextSize);
        t = std::clamp(t, 0.f, 1.f);
        float x0 = textSizeBar.getPosition().x;
        float x1 = x0 + textSizeBar.getSize().x;
        float y = textSizeBar.getPosition().y + 0.5f * textSizeBar.getSize().y;
        float x = x0 + t * (x1 - x0);
        // position is center
        textSizeHandle.setPosition(sf::Vector2f(x, y));
    };
    updateTextSizeHandle();

    // 9) Geometry Helpers
    auto bubbleRect = [&](const SpeechBubble &b)
    {
        return sf::FloatRect(b.getPosition(), b.getSize());
    };

    auto characterRect = [&](const Character &c)
    {
        return sf::FloatRect(c.getPosition(), c.getSize());
    };

    // Resize Handle (Bottom-Right)
    auto handleRect = [&](const sf::FloatRect &r)
    {
        const float h = 10.f;
        return sf::FloatRect(
            sf::Vector2f{r.position.x + r.size.x - h, r.position.y + r.size.y - h},
            sf::Vector2f{h, h});
    };

    // [NEW] Flip Handle (Top-Right)
    auto flipHandleRect = [&](const sf::FloatRect &r)
    {
        const float h = 10.f;
        // Positioned at top-right corner
        return sf::FloatRect(
            sf::Vector2f{r.position.x + r.size.x - h, r.position.y},
            sf::Vector2f{h, h});
    };

    auto mousePositionF = [&](sf::RenderWindow &win)
    {
        auto mp = sf::Mouse::getPosition(win);
        return sf::Vector2f{static_cast<float>(mp.x), static_cast<float>(mp.y)};
    };

    // 10) Brush thickness slider UI
    sf::RectangleShape thicknessBar(sf::Vector2f(SidebarW - 40.f, 4.f));
    thicknessBar.setPosition(sf::Vector2f(
        20.f,
        static_cast<float>(windowHeight) - 140.f));
    thicknessBar.setFillColor(sf::Color(100, 100, 100)); // Dark grey so it's visible on white

    sf::CircleShape thicknessHandle(8.f);
    thicknessHandle.setFillColor(sf::Color(60, 60, 60));

    auto updateThicknessHandle = [&]()
    {
        float t = (currentBrushThickness - minBrushThickness) /
                  (maxBrushThickness - minBrushThickness);
        t = std::clamp(t, 0.f, 1.f);
        float x0 = thicknessBar.getPosition().x;
        float x1 = x0 + thicknessBar.getSize().x;
        float y = thicknessBar.getPosition().y;
        float x = x0 + t * (x1 - x0);
        thicknessHandle.setPosition(sf::Vector2f(
            x - thicknessHandle.getRadius(),
            y - thicknessHandle.getRadius()));
    };
    updateThicknessHandle();

    // 11) Color wheel UI
    const int wheelSize = 140;
    const float wheelRadius = wheelSize * 0.5f;

    sf::Image colorWheelImage(
        sf::Vector2u(static_cast<unsigned>(wheelSize),
                     static_cast<unsigned>(wheelSize)),
        sf::Color::Transparent);

    for (int y = 0; y < wheelSize; ++y)
    {
        for (int x = 0; x < wheelSize; ++x)
        {
            float dx = x + 0.5f - wheelRadius;
            float dy = y + 0.5f - wheelRadius;
            float r = std::sqrt(dx * dx + dy * dy);
            if (r > wheelRadius)
                continue;

            float angle = std::atan2(dy, dx); // [-pi, pi]
            if (angle < 0)
                angle += 2.f * 3.14159265f;

            float h = angle / (2.f * 3.14159265f); // [0,1]
            float s = r / wheelRadius;
            float v = 1.f;

            colorWheelImage.setPixel(
                sf::Vector2u(static_cast<unsigned>(x),
                             static_cast<unsigned>(y)),
                hsvToRgb(h, s, v));
        }
    }

    sf::Texture colorWheelTexture;
    if (!colorWheelTexture.loadFromImage(colorWheelImage))
    {
        std::cerr << "[ColorWheel] Failed to load texture from generated image\n";
    }

    sf::Sprite colorWheelSprite(colorWheelTexture);
    // Position is set in updateLayout

    // ------------------------------------------------------------------------
    // Layout Update Function
    // ------------------------------------------------------------------------
    auto updateLayout = [&]()
    {
        auto sz = window.getSize();
        windowWidth = sz.x;
        windowHeight = sz.y;

        // Sidebar background
        sidebarBg.setSize(sf::Vector2f{SidebarW, static_cast<float>(windowHeight)});

        // 1. Draw Mode Button (Left Bottom)
        drawButton.setPosition({10.f, static_cast<float>(windowHeight) - 50.f});
        auto dbB = drawButton.getLocalBounds();
        auto dbT = drawButtonText.getLocalBounds();
        drawButtonText.setPosition({drawButton.getPosition().x + (dbB.size.x - dbT.size.x) / 2.f,
                                    drawButton.getPosition().y + (dbB.size.y - dbT.size.y) / 2.f - 2.f});

        // 2. Eraser Button (Right Bottom)
        eraserButton.setPosition({15.f + buttonWidth, static_cast<float>(windowHeight) - 50.f});
        auto erB = eraserButton.getLocalBounds();
        auto erT = eraserButtonText.getLocalBounds();
        eraserButtonText.setPosition({eraserButton.getPosition().x + (erB.size.x - erT.size.x) / 2.f,
                                      eraserButton.getPosition().y + (erB.size.y - erT.size.y) / 2.f - 2.f});

        // 3. Undo/Redo Buttons (Above Draw/Eraser)
        undoButton.setPosition({10.f, static_cast<float>(windowHeight) - 100.f});
        redoButton.setPosition({15.f + buttonWidth, static_cast<float>(windowHeight) - 100.f});

        auto ubB = undoButton.getLocalBounds();
        auto ubT = undoButtonText.getLocalBounds();
        undoButtonText.setPosition({undoButton.getPosition().x + (ubB.size.x - ubT.size.x) / 2.f,
                                    undoButton.getPosition().y + (ubB.size.y - ubT.size.y) / 2.f - 2.f});

        auto rbB = redoButton.getLocalBounds();
        auto rbT = redoButtonText.getLocalBounds();
        redoButtonText.setPosition({redoButton.getPosition().x + (rbB.size.x - rbT.size.x) / 2.f,
                                    redoButton.getPosition().y + (rbB.size.y - rbT.size.y) / 2.f - 2.f});

        // 4. Thickness Slider (Above Undo/Redo)
        thicknessBar.setPosition(sf::Vector2f(20.f, static_cast<float>(windowHeight) - 140.f));
        updateThicknessHandle();

        // 5. Export Button (Above Thickness Slider)
        // Export is 40px high + spacing. Let's place it at H - 190.
        exportButton.setPosition({10.f, static_cast<float>(windowHeight) - 390.f});
        auto ebB = exportButton.getLocalBounds();
        auto ebT = exportButtonText.getLocalBounds();
        exportButtonText.setPosition({exportButton.getPosition().x + (ebB.size.x - ebT.size.x) / 2.f,
                                      exportButton.getPosition().y + (ebB.size.y - ebT.size.y) / 2.f - 4.f});

        // 6. Color Wheel (H - 320)
        colorWheelSprite.setPosition(sf::Vector2f((SidebarW - static_cast<float>(wheelSize)) * 0.5f,
                                                  static_cast<float>(windowHeight) - 340.f));
    };

    // Initial layout
    updateLayout();

    // ------------------------------------------------------------------------
    // Main Event Loop
    // ------------------------------------------------------------------------
    while (window.isOpen())
    {
        // Update mouse pos
        sf::Vector2f mpos = mousePositionF(window);

        // Check hover states
        if (exportButton.getGlobalBounds().contains(mpos))
        {
            isExportHovered = true;
        }
        else
        {
            isExportHovered = false;
        }

        if (eraserButton.getGlobalBounds().contains(mpos))
        {
            isEraserHovered = true;
        }
        else
        {
            isEraserHovered = false;
        }

        for (auto evt = window.pollEvent(); evt; evt = window.pollEvent())
        {
            // System Events
            if (evt->is<sf::Event::Closed>())
            {
                window.close();
                continue;
            }

            if (evt->is<sf::Event::Resized>())
            {
                // update SFML view to new size to avoid stretching
                auto s = window.getSize();
                window.setView(sf::View(sf::FloatRect(sf::Vector2f{0.f, 0.f}, sf::Vector2f{static_cast<float>(s.x), static_cast<float>(s.y)})));
                // update stored sizes and UI positions
                updateLayout();
                rebuildPalette();
                continue;
            }

            // Keyboard Shortcuts
            if (evt->is<sf::Event::KeyPressed>())
            {
                const auto *kp = evt->getIf<sf::Event::KeyPressed>();
                auto key = kp->code;

                // Undo: Ctrl+Z
                if (key == sf::Keyboard::Key::Z &&
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
                {
                    commandManager.undo();
                    picked = PickKind::None;
                    pickedIndex = -1;
                    activeBubble = nullptr;
                }

                // Redo: Ctrl+Y
                if (key == sf::Keyboard::Key::Y &&
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
                {
                    commandManager.redo();
                    picked = PickKind::None;
                    pickedIndex = -1;
                    activeBubble = nullptr;
                }

                // Backspace text in active bubble
                if (activeBubble && key == sf::Keyboard::Key::Backspace)
                {
                    auto t = activeBubble->getText();
                    if (!t.empty())
                    {
                        t.pop_back();
                        activeBubble->setText(t);
                    }
                }

                // Delete selected object
                if (key == sf::Keyboard::Key::Delete && pickedIndex >= 0)
                {
                    if (picked == PickKind::Bubble &&
                        pickedIndex < static_cast<int>(bubbles.size()))
                    {
                        auto cmd = std::make_unique<DeleteBubbleCommand>(bubbles, pickedIndex);
                        commandManager.executeCommand(std::move(cmd));
                        activeBubble = nullptr;
                        picked = PickKind::None;
                        pickedIndex = -1;
                    }
                    else if (picked == PickKind::Sprite &&
                             pickedIndex < static_cast<int>(characters.size()))
                    {
                        auto cmd = std::make_unique<DeleteCharacterCommand>(characters, pickedIndex);
                        commandManager.executeCommand(std::move(cmd));
                        picked = PickKind::None;
                        pickedIndex = -1;
                    }
                }

                continue;
            }

            // Text Input
            if (evt->is<sf::Event::TextEntered>())
            {
                if (activeBubble)
                {
                    if (const auto *te = evt->getIf<sf::Event::TextEntered>())
                    {
                        char32_t u = te->unicode;
                        if (u == U'\r' || u == U'\n')
                        {
                            auto t = activeBubble->getText();
                            t.push_back('\n');
                            activeBubble->setText(t);
                        }
                        else if (u >= 32 && u != 127)
                        {
                            auto t = activeBubble->getText();
                            t.push_back(static_cast<char>(u));
                            activeBubble->setText(t);
                        }
                    }
                }
                continue;
            }

            // Mouse Button Pressed
            if (evt->is<sf::Event::MouseButtonPressed>())
            {
                const auto *mb = evt->getIf<sf::Event::MouseButtonPressed>();
                if (!mb)
                    continue;

                mpos = mousePositionF(window);

                if (mb->button == sf::Mouse::Button::Left)
                {
                    // ========================================================
                    // SIDEBAR CLICK HANDLING
                    // ========================================================
                    if (mpos.x <= SidebarW)
                    {
                        // 1. Thickness bar
                        if (thicknessBar.getGlobalBounds().contains(mpos))
                        {
                            draggingThickness = true;
                            float x0 = thicknessBar.getPosition().x;
                            float x1 = x0 + thicknessBar.getSize().x;
                            float t = (mpos.x - x0) / (x1 - x0);
                            t = std::clamp(t, 0.f, 1.f);
                            currentBrushThickness =
                                minBrushThickness + t * (maxBrushThickness - minBrushThickness);
                            updateThicknessHandle();
                            continue;
                        }

                        // 2. Export Button
                        if (exportButton.getGlobalBounds().contains(mpos))
                        {
                            saveNextFrame = true; // Trigger save on next render
                            std::cout << "[Export] Snapshot requested..." << std::endl;
                            continue;
                        }

                        // 3. Eraser Button
                        if (eraserButton.getGlobalBounds().contains(mpos))
                        {
                            eraserActive = !eraserActive;
                            if (eraserActive)
                            {
                                drawMode = true; // Automatically enable draw mode
                                auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Cross);
                                if (cursor)
                                    window.setMouseCursor(*cursor);
                            }
                            std::cout << "[Eraser] " << (eraserActive ? "ON" : "OFF") << std::endl;
                            continue;
                        }

                        // 4. Text size slider
                        if (currentCategory == Category::Fonts && textSizeBar.getGlobalBounds().contains(mpos))
                        {
                            draggingTextSize = true;
                            textSizeOld = activeBubble ? activeBubble->getFontSize() : -1;

                            float x0 = textSizeBar.getPosition().x;
                            float x1 = x0 + textSizeBar.getSize().x;
                            float t = (mpos.x - x0) / (x1 - x0);
                            t = std::clamp(t, 0.f, 1.f);
                            currentTextSize = minTextSize + t * (maxTextSize - minTextSize);
                            updateTextSizeHandle();

                            if (activeBubble)
                            {
                                activeBubble->setFontSize(static_cast<int>(currentTextSize));
                            }

                            continue;
                        }

                        // 5. Color wheel
                        if (colorWheelSprite.getGlobalBounds().contains(mpos))
                        {
                            pickingColor = true;
                            eraserActive = false; // Picking a color disables eraser
                            sf::Vector2f local = mpos - colorWheelSprite.getPosition();
                            int px = static_cast<int>(local.x);
                            int py = static_cast<int>(local.y);
                            if (px >= 0 && px < wheelSize && py >= 0 && py < wheelSize)
                            {
                                sf::Color c = colorWheelImage.getPixel(
                                    sf::Vector2u(static_cast<unsigned>(px),
                                                 static_cast<unsigned>(py)));
                                if (c.a != 0)
                                {
                                    currentBrushColor = c;
                                }
                            }

                            continue;
                        }

                        // 6. Draw button
                        if (drawButton.getGlobalBounds().contains(mpos))
                        {
                            drawMode = !drawMode;
                            if (!drawMode)
                            {
                                eraserActive = false; // Disable eraser if turning off draw mode
                            }
                            activeStroke = nullptr;

                            if (drawMode)
                            {
                                auto cursor =
                                    sf::Cursor::createFromSystem(sf::Cursor::Type::Cross);
                                if (cursor)
                                {
                                    window.setMouseCursor(*cursor);
                                }
                                drawButton.setFillColor(sf::Color(120, 220, 120));
                            }
                            else
                            {
                                auto cursor =
                                    sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
                                if (cursor)
                                {
                                    window.setMouseCursor(*cursor);
                                }
                                drawButton.setFillColor(sf::Color(200, 200, 200));
                            }

                            std::cout << "[DrawMode] " << (drawMode ? "ON" : "OFF") << "\n";
                            continue;
                        }

                        // 7. Undo button
                        if (undoButton.getGlobalBounds().contains(mpos))
                        {
                            commandManager.undo();
                            picked = PickKind::None;
                            pickedIndex = -1;
                            activeBubble = nullptr;
                            continue;
                        }

                        // 8. Redo button
                        if (redoButton.getGlobalBounds().contains(mpos))
                        {
                            commandManager.redo();
                            picked = PickKind::None;
                            pickedIndex = -1;
                            activeBubble = nullptr;
                            continue;
                        }

                        // 9. Category headers
                        bool headerHit = false;
                        for (const auto &h : headers)
                        {
                            if (h.hit.contains(mpos))
                            {
                                if (currentCategory != h.category)
                                {
                                    currentCategory = h.category;
                                    rebuildPalette();
                                }
                                headerHit = true;
                                break;
                            }
                        }
                        if (headerHit)
                            continue;

                        // 10. Palette items
                        for (const auto &item : palette)
                        {
                            if (!item.hit.contains(mpos))
                                continue;

                            if (item.assetType == "CHARACTER")
                            {
                                if (auto tex = AM.getTexture(item.assetKey))
                                {
                                    auto ch = std::make_unique<Character>(
                                        item.assetKey,
                                        item.assetKey,
                                        SidebarW + 24.f, 24.f,
                                        220.f, 220.f);
                                    auto cmd = std::make_unique<AddCharacterCommand>(
                                        characters, std::move(ch));
                                    commandManager.executeCommand(std::move(cmd));

                                    activeBubble = nullptr;
                                    picked = PickKind::Sprite;
                                    pickedIndex = static_cast<int>(characters.size()) - 1;
                                }
                            }
                            else if (item.assetType == "BUBBLE")
                            {
                                auto b = std::make_unique<SpeechBubble>(
                                    "spawn", "",
                                    SidebarW + 24.f, 24.f,
                                    280.f, 120.f);
                                b->setStyle(item.assetKey);
                                auto cmd = std::make_unique<AddBubbleCommand>(
                                    bubbles, std::move(b));
                                commandManager.executeCommand(std::move(cmd));

                                activeBubble = bubbles.empty() ? nullptr : bubbles.back().get();
                                picked = PickKind::Bubble;
                                pickedIndex = static_cast<int>(bubbles.size()) - 1;
                            }
                            else if (item.assetType == "FONT")
                            {
                                if (activeBubble)
                                {
                                    activeBubble->setFontName(item.assetKey);
                                    std::cout << "[Font] Changed to: " << item.assetKey << "\n";
                                }
                                else
                                {
                                    std::cout << "[Font] No bubble selected.\n";
                                }
                            }

                            // Palette click handled
                            break;
                        }

                        continue; // sidebar handled
                    }

                    // ========================================================
                    // CANVAS CLICK HANDLING
                    // ========================================================
                    if (mpos.x > SidebarW)
                    {
                    }

                    if (drawMode && mpos.x > SidebarW)
                    {
                        // Start new stroke
                        // Use White if Eraser is active, else use selected color
                        sf::Color brushColor = eraserActive ? sf::Color::White : currentBrushColor;

                        std::string id = "stroke_" + std::to_string(strokes.size() + 1);
                        auto stroke = std::make_unique<BrushStroke>(
                            id, brushColor, currentBrushThickness);
                        activeStroke = stroke.get();
                        activeStroke->beginAt(mpos);

                        auto cmd = std::make_unique<AddStrokeCommand>(strokes, std::move(stroke));
                        commandManager.executeCommand(std::move(cmd));
                        continue;
                    }

                    // If not in draw mode, handle selection / dragging / resizing
                    picked = PickKind::None;
                    pickedIndex = -1;
                    activeBubble = nullptr;

                    bool hit = false;

                    // [NEW] CHECK FLIP HANDLE CLICK (Top-Right)
                    for (int i = static_cast<int>(bubbles.size()) - 1; i >= 0 && !hit; --i)
                    {
                        if (flipHandleRect(bubbleRect(*bubbles[i])).contains(mpos))
                        {
                            bubbles[i]->setFlipped(!bubbles[i]->isFlipped());
                            picked = PickKind::Bubble;
                            pickedIndex = i;
                            activeBubble = bubbles[i].get();
                            hit = true;
                        }
                    }
                    if (!hit)
                    {
                        for (int i = static_cast<int>(characters.size()) - 1; i >= 0 && !hit; --i)
                        {
                            if (flipHandleRect(characterRect(*characters[i])).contains(mpos))
                            {
                                characters[i]->setFlipped(!characters[i]->isFlipped());
                                picked = PickKind::Sprite;
                                pickedIndex = i;
                                hit = true;
                            }
                        }
                    }
                    if (hit)
                        continue;

                    // Resize handles for bubbles
                    for (int i = static_cast<int>(bubbles.size()) - 1; i >= 0 && !hit; --i)
                    {
                        if (handleRect(bubbleRect(*bubbles[i])).contains(mpos))
                        {
                            resizing = true;
                            resizeKind = PickKind::Bubble;
                            resizeIndex = i;
                            resizeStartMouse = mpos;
                            resizeStartSize = bubbles[i]->getSize();
                            resizeStartPos = bubbles[i]->getPosition();
                            activeBubble = bubbles[i].get();
                            picked = PickKind::Bubble;
                            pickedIndex = i;
                            hit = true;
                        }
                    }

                    // Resize handles for characters
                    for (int i = static_cast<int>(characters.size()) - 1; i >= 0 && !hit; --i)
                    {
                        if (handleRect(characterRect(*characters[i])).contains(mpos))
                        {
                            resizing = true;
                            resizeKind = PickKind::Sprite;
                            resizeIndex = i;
                            resizeStartMouse = mpos;
                            resizeStartSize = characters[i]->getSize();
                            resizeStartPos = characters[i]->getPosition();
                            picked = PickKind::Sprite;
                            pickedIndex = i;
                            hit = true;
                        }
                    }

                    if (hit)
                        continue;

                    // Drag characters
                    for (int i = static_cast<int>(characters.size()) - 1; i >= 0; --i)
                    {
                        if (characterRect(*characters[i]).contains(mpos))
                        {
                            draggingSprite = true;
                            dragSpriteIdx = i;
                            dragOffset = mpos - characters[i]->getPosition();
                            picked = PickKind::Sprite;
                            pickedIndex = i;
                            hit = true;
                            break;
                        }
                    }

                    // Drag bubbles
                    if (!draggingSprite)
                    {
                        for (int i = static_cast<int>(bubbles.size()) - 1; i >= 0; --i)
                        {
                            auto pos = bubbles[i]->getPosition();
                            auto size = bubbles[i]->getSize();
                            if (mpos.x >= pos.x && mpos.x <= pos.x + size.x &&
                                mpos.y >= pos.y && mpos.y <= pos.y + size.y)
                            {
                                draggingBubble = true;
                                dragBubbleIdx = i;
                                dragOffset = mpos - pos;
                                activeBubble = bubbles[i].get();
                                picked = PickKind::Bubble;
                                pickedIndex = i;
                                break;
                            }
                        }
                    }
                }

                continue;
            }

            // Mouse button released
            if (evt->is<sf::Event::MouseButtonReleased>())
            {
                draggingThickness = false;
                pickingColor = false;

                // If we were dragging the text-size slider
                if (draggingTextSize)
                {
                    int newSize = static_cast<int>(currentTextSize);
                    if (textSizeOld != -1 && activeBubble && textSizeOld != newSize)
                    {
                        auto cmd = std::make_unique<ChangeBubbleFontSizeCommand>(
                            activeBubble, textSizeOld, newSize);
                        commandManager.executeCommand(std::move(cmd));
                    }
                    draggingTextSize = false;
                    textSizeOld = -1;
                }

                // End stroke
                activeStroke = nullptr;

                resizing = false;
                resizeKind = PickKind::None;
                resizeIndex = -1;
                draggingSprite = false;
                draggingBubble = false;
                dragSpriteIdx = -1;
                dragBubbleIdx = -1;

                continue;
            }

            // Mouse moved
            if (evt->is<sf::Event::MouseMoved>())
            {
                sf::Vector2f mpos = mousePositionF(window);

                // Brush: change thickness while dragging slider
                if (draggingThickness)
                {
                    float x0 = thicknessBar.getPosition().x;
                    float x1 = x0 + thicknessBar.getSize().x;
                    float t = (mpos.x - x0) / (x1 - x0);
                    t = std::clamp(t, 0.f, 1.f);
                    currentBrushThickness =
                        minBrushThickness + t * (maxBrushThickness - minBrushThickness);
                    updateThicknessHandle();
                }

                // Text size slider dragging
                if (draggingTextSize && activeBubble)
                {
                    float x0 = textSizeBar.getPosition().x;
                    float x1 = x0 + textSizeBar.getSize().x;
                    float t = (mpos.x - x0) / (x1 - x0);
                    t = std::clamp(t, 0.f, 1.f);
                    currentTextSize = minTextSize + t * (maxTextSize - minTextSize);
                    updateTextSizeHandle();

                    int newSize = static_cast<int>(currentTextSize);
                    activeBubble->setFontSize(newSize);
                }

                // Hover detection for text-size slider handle
                if (currentCategory == Category::Fonts && !draggingTextSize)
                {
                    sf::Vector2f handlePos = textSizeHandle.getPosition();
                    float dist = std::sqrt((mpos.x - handlePos.x) * (mpos.x - handlePos.x) +
                                           (mpos.y - handlePos.y) * (mpos.y - handlePos.y));
                    float hoverRadius = 16.f; // Hover detection radius

                    if (dist < hoverRadius && mpos.x <= SidebarW)
                    {
                        if (!hoverTextSizeSlider)
                        {
                            hoverTextSizeSlider = true;
                            hoverClock.restart();
                        }
                    }
                    else
                    {
                        hoverTextSizeSlider = false;
                    }
                }
                else if (draggingTextSize)
                {
                    hoverTextSizeSlider = true; // Keep hover true while dragging
                }
                else
                {
                    hoverTextSizeSlider = false;
                }

                // Brush: change color while dragging on wheel
                if (pickingColor)
                {
                    sf::Vector2f local = mpos - colorWheelSprite.getPosition();
                    int px = static_cast<int>(local.x);
                    int py = static_cast<int>(local.y);
                    if (px >= 0 && px < wheelSize && py >= 0 && py < wheelSize)
                    {
                        sf::Color c = colorWheelImage.getPixel(
                            sf::Vector2u(static_cast<unsigned>(px),
                                         static_cast<unsigned>(py)));
                        if (c.a != 0)
                        {
                            currentBrushColor = c;
                        }
                    }
                }

                // Draw mode: extend active stroke (only if not over sidebar)
                if (drawMode && activeStroke && mpos.x > SidebarW)
                {
                    activeStroke->addPoint(mpos);
                    continue;
                }

                // Resizing
                if (resizing && resizeIndex >= 0)
                {
                    sf::Vector2f delta = mpos - resizeStartMouse;
                    sf::Vector2f newSize = resizeStartSize + delta;
                    newSize.x = std::max(newSize.x, 60.f);
                    newSize.y = std::max(newSize.y, 40.f);

                    if (resizeKind == PickKind::Bubble)
                    {
                        bubbles[resizeIndex]->setSize(newSize.x, newSize.y);
                        bubbles[resizeIndex]->setPosition(resizeStartPos.x, resizeStartPos.y);
                    }
                    else if (resizeKind == PickKind::Sprite)
                    {
                        characters[resizeIndex]->setSize(newSize.x, newSize.y);
                        characters[resizeIndex]->setPosition(resizeStartPos.x, resizeStartPos.y);
                    }
                }

                // Drag characters
                if (draggingSprite && dragSpriteIdx >= 0)
                {
                    sf::Vector2f newPos = mpos - dragOffset;
                    characters[dragSpriteIdx]->setPosition(newPos.x, newPos.y);
                }

                // Drag bubbles
                if (draggingBubble && dragBubbleIdx >= 0)
                {
                    sf::Vector2f newPos = mpos - dragOffset;
                    bubbles[dragBubbleIdx]->setPosition(newPos.x, newPos.y);
                }

                continue;
            }
        }

        // --------------------------------------------------------------------
        // RENDER LOOP
        // --------------------------------------------------------------------
        window.clear(sf::Color::White);

        // 1. Draw Scene Objects
        for (const auto &s : strokes)
        {
            s->draw(window);
        }
        for (const auto &c : characters)
        {
            c->draw(window);
        }
        for (const auto &b : bubbles)
        {
            b->draw(window);
        }

        // 2. Handle Export (Capture Scene Only)
        if (saveNextFrame)
        {
            sf::Texture texture;
            if (texture.resize(window.getSize())) // Check return for nodiscard
            {
                texture.update(window);
                sf::Image screenshot = texture.copyToImage();

                // Crop out the sidebar (Start from SidebarW)
                unsigned int cropX = static_cast<unsigned int>(SidebarW);
                unsigned int cropW = window.getSize().x - cropX;
                unsigned int cropH = window.getSize().y;

                // Define output folder
                namespace fs = std::filesystem;
                std::string exportDir = "SavedComics";

                // Create directory if it doesn't exist
                if (!fs::exists(exportDir))
                {
                    fs::create_directory(exportDir);
                }

                if (cropW > 0 && cropH > 0)
                {
                    sf::Image finalImage;
                    // SFML 3: resize takes Vector2u, returns void
                    finalImage.resize(sf::Vector2u(cropW, cropH));

                    // SFML 3: copy takes Vector2u for dest, IntRect for source
                    if (finalImage.copy(
                            screenshot,
                            sf::Vector2u(0, 0),
                            sf::IntRect(sf::Vector2i(static_cast<int>(cropX), 0),
                                        sf::Vector2i(static_cast<int>(cropW), static_cast<int>(cropH)))))
                    {
                        std::string filename = exportDir + "/Comic_" + std::to_string(std::time(nullptr)) + ".png";

                        if (finalImage.saveToFile(filename))
                        {
                            std::cout << "[Export] Success! Saved to: " << filename << std::endl;
                        }
                        else
                        {
                            std::cerr << "[Export] Failed to save image." << std::endl;
                        }
                    }
                }
            }
            saveNextFrame = false;
        }

        // 3. Draw UI Layer (Sidebar Background)
        window.draw(sidebarBg);

        // 4. Draw Headers
        for (const auto &h : headers)
        {
            sf::RectangleShape rect;
            rect.setPosition(h.hit.position);
            rect.setSize(h.hit.size);
            rect.setFillColor((h.category == currentCategory)
                                  ? sf::Color(210, 210, 210)
                                  : sf::Color(235, 235, 235));
            rect.setOutlineColor(sf::Color(180, 180, 180));
            rect.setOutlineThickness(1.f);
            window.draw(rect);

            sf::Text label(AM.getFont("actionman"));
            label.setCharacterSize(14);
            label.setFillColor(sf::Color::Black);
            label.setString(
                h.category == Category::Characters ? "Characters"
                                                   : (h.category == Category::Fonts ? "Fonts"
                                                                                    : "Bubbles"));
            auto bounds = label.getLocalBounds();
            label.setPosition({h.hit.position.x + 8.f,
                               h.hit.position.y + 0.5f * (h.hit.size.y - bounds.size.y)});
            window.draw(label);
        }

        // 5. Draw Palette Items
        for (const auto &row : palette)
        {
            sf::RectangleShape r;
            r.setPosition(row.hit.position);
            r.setSize(row.hit.size);
            r.setFillColor(sf::Color(245, 245, 245));
            r.setOutlineColor(sf::Color(180, 180, 180));
            r.setOutlineThickness(1.f);
            window.draw(r);

            sf::Vector2f pad{12.f, 10.f};
            sf::Vector2f boxTL = row.hit.position + pad;
            sf::Vector2f boxSize = row.hit.size - sf::Vector2f{pad.x * 2.f, pad.y * 2.f};

            if (row.assetType == "CHARACTER")
            {
                if (auto tex = AM.getTexture(row.assetKey))
                {
                    sf::Sprite s(*tex);
                    float sc = std::min(
                        boxSize.x / static_cast<float>(tex->getSize().x),
                        boxSize.y / static_cast<float>(tex->getSize().y));
                    sf::Vector2f sprSize{
                        tex->getSize().x * sc,
                        tex->getSize().y * sc};
                    s.setScale({sc, sc});
                    s.setPosition(boxTL + 0.5f * (boxSize - sprSize));
                    window.draw(s);
                }
            }
            else if (row.assetType == "BUBBLE")
            {
                if (auto tex = AM.getTexture("bubble_" + row.assetKey))
                {
                    sf::Sprite preview(*tex);
                    float scale = std::min(
                        boxSize.x / static_cast<float>(tex->getSize().x),
                        boxSize.y / static_cast<float>(tex->getSize().y));
                    sf::Vector2f previewSize{
                        tex->getSize().x * scale,
                        tex->getSize().y * scale};
                    preview.setScale({scale, scale});
                    preview.setPosition(boxTL + 0.5f * (boxSize - previewSize));
                    window.draw(preview);
                }
            }
            else if (row.assetType == "FONT")
            {
                sf::Text t(AM.getFont(row.assetKey));
                t.setString("Aa");
                t.setCharacterSize(24);
                t.setFillColor(sf::Color::Black);
                auto bounds = t.getLocalBounds();
                t.setPosition({boxTL.x,
                               boxTL.y + 0.5f * (boxSize.y - bounds.size.y)});
                window.draw(t);

                // Store font section bounds for slider positioning below all fonts
                fontSectionBounds = row.hit;
            }
        }

        // 6. Text Size Slider (Conditional)
        if (currentCategory == Category::Fonts && fontSectionBounds.size.y > 0)
        {
            textSizeBar.setPosition(sf::Vector2f(20.f, fontSectionBounds.position.y + fontSectionBounds.size.y + 10.f));
            updateTextSizeHandle();
            window.draw(textSizeBar);

            // Update handle scale with smooth animation on hover/drag
            if (hoverTextSizeSlider || draggingTextSize)
            {
                float targetScale = 1.35f;                          // Scale up on hover/drag
                handleScale += (targetScale - handleScale) * 0.15f; // Smooth easing
            }
            else
            {
                float targetScale = 1.0f;
                handleScale += (targetScale - handleScale) * 0.15f;
            }

            // Draw handle with animation scale and highlight when active
            sf::CircleShape animatedHandle = textSizeHandle;
            animatedHandle.setScale(sf::Vector2f(handleScale, handleScale));

            if (draggingTextSize)
            {
                animatedHandle.setFillColor(sf::Color(40, 160, 240));
                animatedHandle.setOutlineColor(sf::Color::White);
                animatedHandle.setOutlineThickness(2.f);
            }
            else if (hoverTextSizeSlider)
            {
                animatedHandle.setFillColor(sf::Color(100, 140, 200));
                animatedHandle.setOutlineColor(sf::Color(80, 120, 180));
                animatedHandle.setOutlineThickness(1.f);
            }
            else
            {
                animatedHandle.setFillColor(sf::Color(60, 60, 60));
                animatedHandle.setOutlineThickness(0.f);
            }
            window.draw(animatedHandle);

            // Tooltip on hover/drag
            if (hoverTextSizeSlider || draggingTextSize)
            {
                sf::RectangleShape tooltip(sf::Vector2f(50.f, 24.f));
                sf::Vector2f handlePos = textSizeHandle.getPosition();
                tooltip.setPosition(sf::Vector2f(handlePos.x - 25.f, handlePos.y - 30.f));
                tooltip.setFillColor(sf::Color(40, 40, 40));
                tooltip.setOutlineColor(sf::Color::White);
                tooltip.setOutlineThickness(1.f);
                window.draw(tooltip);

                sf::Text tooltipText(AM.getFont("actionman"));
                tooltipText.setCharacterSize(11);
                tooltipText.setFillColor(sf::Color::White);
                tooltipText.setString(std::to_string(static_cast<int>(currentTextSize)) + "pt");
                auto tbounds = tooltipText.getLocalBounds();
                tooltipText.setPosition(sf::Vector2f(
                    tooltip.getPosition().x + (tooltip.getSize().x - tbounds.size.x) / 2.f,
                    tooltip.getPosition().y + (tooltip.getSize().y - tbounds.size.y) / 2.f - 2.f));
                window.draw(tooltipText);
            }

            // Text size value label (live update)
            sf::Text sizeLabel(AM.getFont("actionman"));
            sizeLabel.setCharacterSize(12);
            sizeLabel.setFillColor(sf::Color::Black);
            sizeLabel.setString(std::to_string(static_cast<int>(currentTextSize)) + " px");
            sizeLabel.setPosition(sf::Vector2f(20.f + textSizeBar.getSize().x + 6.f,
                                               fontSectionBounds.position.y + fontSectionBounds.size.y + 4.f));
            window.draw(sizeLabel);
        }

        // 7. Color Wheel & Preview
        window.draw(colorWheelSprite);

        sf::RectangleShape colorPreview(sf::Vector2f(24.f, 24.f));
        colorPreview.setFillColor(currentBrushColor);
        colorPreview.setOutlineColor(sf::Color::Black);
        colorPreview.setOutlineThickness(1.f);
        // Position at the leftmost edge of sidebar and horizontally aligned with center of color wheel
        float wheelCenterY = static_cast<float>(windowHeight) - 340.f + static_cast<float>(wheelSize) * 0.5f;
        colorPreview.setPosition(sf::Vector2f(
            160.f,
            wheelCenterY + 50.f));
        window.draw(colorPreview);

        // 8. Thickness Slider
        window.draw(thicknessBar);
        window.draw(thicknessHandle);

        // 9. Draw Button
        if (drawMode)
        {
            drawButton.setFillColor(sf::Color(120, 220, 120));
        }
        else
        {
            drawButton.setFillColor(sf::Color(200, 200, 200));
        }
        window.draw(drawButton);
        auto drawTextBounds = drawButtonText.getLocalBounds();
        drawButtonText.setPosition({drawButton.getPosition().x +
                                        (drawButton.getSize().x - drawTextBounds.size.x) / 2.f,
                                    drawButton.getPosition().y +
                                        (drawButton.getSize().y - drawTextBounds.size.y) / 2.f - 2.f});
        window.draw(drawButtonText);

        // 10. Eraser Button (Right of Draw)
        if (eraserActive)
        {
            eraserButton.setFillColor(sf::Color(120, 220, 120));
        }
        else if (isEraserHovered)
        {
            eraserButton.setFillColor(sf::Color(220, 220, 220));
        }
        else
        {
            eraserButton.setFillColor(sf::Color(200, 200, 200));
        }
        window.draw(eraserButton);
        auto eraserTextBounds = eraserButtonText.getLocalBounds();
        eraserButtonText.setPosition({eraserButton.getPosition().x +
                                          (eraserButton.getSize().x - eraserTextBounds.size.x) / 2.f,
                                      eraserButton.getPosition().y +
                                          (eraserButton.getSize().y - eraserTextBounds.size.y) / 2.f - 2.f});
        window.draw(eraserButtonText);

        // 11. Undo/Redo buttons
        undoButton.setFillColor(
            commandManager.canUndo() ? sf::Color(200, 200, 200)
                                     : sf::Color(150, 150, 150));
        redoButton.setFillColor(
            commandManager.canRedo() ? sf::Color(200, 200, 200)
                                     : sf::Color(150, 150, 150));

        window.draw(undoButton);
        window.draw(redoButton);

        auto undoTextBounds = undoButtonText.getLocalBounds();
        undoButtonText.setPosition({undoButton.getPosition().x +
                                        (undoButton.getSize().x - undoTextBounds.size.x) / 2.f,
                                    undoButton.getPosition().y +
                                        (undoButton.getSize().y - undoTextBounds.size.y) / 2.f - 2.f});
        window.draw(undoButtonText);

        auto redoTextBounds = redoButtonText.getLocalBounds();
        redoButtonText.setPosition({redoButton.getPosition().x +
                                        (redoButton.getSize().x - redoTextBounds.size.x) / 2.f,
                                    redoButton.getPosition().y +
                                        (redoButton.getSize().y - redoTextBounds.size.y) / 2.f - 2.f});
        window.draw(redoButtonText);

        // 12. Export Button (Restored to position above thickness slider)
        if (saveNextFrame)
        {
            exportButton.setFillColor(sf::Color(120, 220, 120)); // Green
        }
        else if (isExportHovered)
        {
            exportButton.setFillColor(sf::Color(220, 220, 220));
        }
        else
        {
            exportButton.setFillColor(sf::Color(200, 200, 200));
        }

        window.draw(exportButton);
        auto exportTextBounds = exportButtonText.getLocalBounds();
        exportButtonText.setPosition({exportButton.getPosition().x +
                                          (exportButton.getSize().x - exportTextBounds.size.x) / 2.f,
                                      exportButton.getPosition().y +
                                          (exportButton.getSize().y - exportTextBounds.size.y) / 2.f - 4.f});
        window.draw(exportButtonText);

        // 13. Draw Handles (Resize and Flip)
        // Resize Handle (Bottom-Right)
        auto drawResizeHandle = [&](const sf::FloatRect &r)
        {
            auto hr = handleRect(r);
            sf::RectangleShape h;
            h.setPosition(hr.position);
            h.setSize(hr.size);
            h.setFillColor(sf::Color(60, 60, 60)); // Dark Grey
            window.draw(h);
        };

        // Flip Handle (Top-Right)
        auto drawFlipHandle = [&](const sf::FloatRect &r)
        {
            auto hr = flipHandleRect(r);
            sf::RectangleShape h;
            h.setPosition(hr.position);
            h.setSize(hr.size);
            h.setFillColor(sf::Color(0, 200, 255)); // Cyan color for flip
            window.draw(h);
        };

        if (picked == PickKind::Sprite &&
            pickedIndex >= 0 &&
            pickedIndex < static_cast<int>(characters.size()))
        {
            sf::FloatRect r = characterRect(*characters[pickedIndex]);
            drawResizeHandle(r);
            drawFlipHandle(r);
        }
        else if (picked == PickKind::Bubble &&
                 pickedIndex >= 0 &&
                 pickedIndex < static_cast<int>(bubbles.size()))
        {
            sf::FloatRect r = bubbleRect(*bubbles[pickedIndex]);
            drawResizeHandle(r);
            drawFlipHandle(r);
        }

        window.display();
    }

    return 0;
}