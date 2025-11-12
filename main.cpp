// main.cpp
// Comic Strip Maker - Phase 1 (label-free sidebar, typing + delete, no Shout bubble)
// - Sidebar palette with 3 items: Hero, Villain, Speech (no labels)
// - Spawn items by clicking palette rows
// - Drag sprites and bubbles; resize via bottom-right 10x10 handle
// - Type into the selected Speech bubble; Backspace deletes last char; Enter inserts newline
// - Delete key removes the selected bubble from the scene
//
// Cross-version events:
// Uses SFML 3 is<T>/get<T> when SFML_VERSION_MAJOR >= 3, else SFML 2 evt.type path.

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>

#include "AssetManager.h"
#include "SpeechBubble.h"
#include "VectorUtils.h"

enum class ItemKind
{
    Hero,
    Villain,
    Speech
};

struct PaletteItem
{
    ItemKind kind;
    sf::FloatRect hit; // SFML 3: position+size; SFML 2: left/top/width/height (we only use contains)
};

enum class PickKind
{
    None,
    Sprite,
    Bubble
};

int main()
{
    // 1) Load assets
    try
    {
        auto &AM = AssetManager::getInstance();
        AM.loadTexture("hero_face", "assets/1.png");
        AM.loadTexture("villain_face", "assets/2.png");
        AM.loadFont("main_font", "assets/actionman.ttf");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to load assets: " << e.what() << "\n";
        return 1;
    }

    // 2) Window and sidebar
    sf::RenderWindow window(sf::VideoMode({1280, 720}),
                            sf::String("Comic Strip Maker - Phase 1"));
    window.setFramerateLimit(60);

    const float SidebarW = 200.f;
    sf::RectangleShape sidebarBg(sf::Vector2f{SidebarW, 720.f});
    sidebarBg.setFillColor(sf::Color(230, 230, 230));
    sidebarBg.setOutlineColor(sf::Color(200, 200, 200));
    sidebarBg.setOutlineThickness(1.f);

    // 3) Palette (no labels)
    std::vector<PaletteItem> palette = {
        {ItemKind::Hero, sf::FloatRect{sf::Vector2f{16.f, 24.f}, sf::Vector2f{SidebarW - 32.f, 60.f}}},
        {ItemKind::Villain, sf::FloatRect{sf::Vector2f{16.f, 94.f}, sf::Vector2f{SidebarW - 32.f, 60.f}}},
        {ItemKind::Speech, sf::FloatRect{sf::Vector2f{16.f, 164.f}, sf::Vector2f{SidebarW - 32.f, 60.f}}},
    };

    // 4) Scene containers
    std::vector<sf::Sprite> sprites;
    std::vector<std::unique_ptr<SpeechBubble>> bubbles;

    // 5) Spawners
    auto spawnSprite = [&](ItemKind k, sf::Vector2f at)
    {
        const char *key = (k == ItemKind::Hero) ? "hero_face" : "villain_face";
        if (auto t = AssetManager::getInstance().getTexture(key))
        {
            sprites.emplace_back(*t);
            auto &s = sprites.back();
            t->setSmooth(true);
            float targetH = 128.f;
            float sh = targetH / t->getSize().y;
            s.setScale({sh, sh});
            s.setPosition(at);
        }
    };

    auto spawnBubble = [&](sf::Vector2f at)
    {
        auto b = std::make_unique<SpeechBubble>("spawn", "", at.x, at.y, 280.f, 100.f);
        b->setStyle("speech");
        bubbles.emplace_back(std::move(b));
    };

    // 6) Interaction state
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
    sf::Vector2f resizeStartScale{1.f, 1.f};
    sf::Vector2f resizeStartPos{0.f, 0.f};

    // 7) Helpers
    auto spriteBounds = [&](const sf::Sprite &s)
    { return s.getGlobalBounds(); };
    auto bubbleRect = [&](const SpeechBubble &b)
    {
        auto p = b.getPosition();
        auto sz = b.getSize();
        return sf::FloatRect(sf::Vector2f{p.x, p.y}, sf::Vector2f{sz.x, sz.y});
    };
    auto handleRect = [&](const sf::FloatRect &r)
    {
        const float h = 10.f;
        return sf::FloatRect(
#if (SFML_VERSION_MAJOR >= 3)
            sf::Vector2f{r.position.x + r.size.x - h, r.position.y + r.size.y - h},
            sf::Vector2f{h, h}
#else
            r.left + r.width - h, r.top + r.height - h, h, h
#endif
        );
    };

    // 8) Main loop
    while (window.isOpen())
    {

#if (SFML_VERSION_MAJOR >= 3)
        // ------------------------- SFML 3 events -------------------------
        for (auto evt = window.pollEvent(); evt; evt = window.pollEvent())
        {
            if (evt->is<sf::Event::Closed>())
                window.close();

            if (evt->is<sf::Event::MouseButtonPressed>())
            {
                auto mp = sf::Mouse::getPosition(window);
                sf::Vector2f mpos{static_cast<float>(mp.x), static_cast<float>(mp.y)};

                // Sidebar spawn
                if (mpos.x <= SidebarW)
                {
                    for (const auto &it : palette)
                    {
                        if (it.hit.contains(mpos))
                        {
                            if (it.kind == ItemKind::Hero || it.kind == ItemKind::Villain)
                            {
                                spawnSprite(it.kind, {SidebarW + 24.f, 24.f});
                                activeBubble = nullptr;
                                picked = PickKind::Sprite;
                                pickedIndex = static_cast<int>(sprites.size()) - 1;
                            }
                            else
                            {
                                spawnBubble({SidebarW + 24.f, 24.f});
                                activeBubble = bubbles.empty() ? nullptr : bubbles.back().get();
                                picked = PickKind::Bubble;
                                pickedIndex = static_cast<int>(bubbles.size()) - 1;
                            }
                            break;
                        }
                    }
                    continue;
                }

                // Clear selection; reassign below if something is hit
                picked = PickKind::None;
                pickedIndex = -1;
                activeBubble = nullptr;

                // Try resize handles (bubbles then sprites, topmost-first)
                bool hit = false;
                for (int i = static_cast<int>(bubbles.size()) - 1; i >= 0 && !hit; --i)
                {
                    auto r = bubbleRect(*bubbles[i]);
                    if (handleRect(r).contains(mpos))
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
                for (int i = static_cast<int>(sprites.size()) - 1; i >= 0 && !hit; --i)
                {
                    auto r = spriteBounds(sprites[i]);
                    if (handleRect(r).contains(mpos))
                    {
                        resizing = true;
                        resizeKind = PickKind::Sprite;
                        resizeIndex = i;
                        resizeStartMouse = mpos;
                        resizeStartScale = sprites[i].getScale();
                        resizeStartPos = sprites[i].getPosition();
                        picked = PickKind::Sprite;
                        pickedIndex = i;
                        hit = true;
                    }
                }
                if (hit)
                    continue;

                // Drag sprites
                draggingSprite = false;
                dragSpriteIdx = -1;
                for (int i = static_cast<int>(sprites.size()) - 1; i >= 0; --i)
                {
                    if (sprites[i].getGlobalBounds().contains(mpos))
                    {
                        draggingSprite = true;
                        dragSpriteIdx = i;
                        dragOffset = mpos - sprites[i].getPosition();
                        picked = PickKind::Sprite;
                        pickedIndex = i;
                        break;
                    }
                }

                // Drag bubbles
                if (!draggingSprite)
                {
                    draggingBubble = false;
                    dragBubbleIdx = -1;
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

            if (evt->is<sf::Event::MouseButtonReleased>())
            {
                resizing = false;
                resizeKind = PickKind::None;
                resizeIndex = -1;
                draggingSprite = draggingBubble = false;
                dragSpriteIdx = dragBubbleIdx = -1;
            }

            if (evt->is<sf::Event::MouseMoved>())
            {
                auto mp = sf::Mouse::getPosition(window);
                sf::Vector2f mpos{static_cast<float>(mp.x), static_cast<float>(mp.y)};

                if (resizing && resizeIndex >= 0)
                {
                    sf::Vector2f delta = mpos - resizeStartMouse;
                    if (resizeKind == PickKind::Bubble)
                    {
                        sf::Vector2f newSize = resizeStartSize + sf::Vector2f{delta.x, delta.y};
                        newSize.x = std::max(newSize.x, 60.f);
                        newSize.y = std::max(newSize.y, 40.f);
                        bubbles[resizeIndex]->setSize(newSize);
                        bubbles[resizeIndex]->setPosition(resizeStartPos);
                    }
                    else if (resizeKind == PickKind::Sprite)
                    {
                        float factor = 1.f + (delta.x / 200.f);
                        factor = std::max(0.25f, std::min(4.f, factor));
                        auto base = resizeStartScale;
                        sprites[resizeIndex].setScale({base.x * factor, base.y * factor});
                        sprites[resizeIndex].setPosition(resizeStartPos);
                    }
                }

                if (draggingSprite && dragSpriteIdx >= 0)
                {
                    auto pos = mpos - dragOffset;
                    sprites[dragSpriteIdx].setPosition(pos);
                }
                if (draggingBubble && dragBubbleIdx >= 0)
                {
                    auto pos = mpos - dragOffset;
                    bubbles[dragBubbleIdx]->setPosition(pos);
                }
            }

            // Text input into active bubble
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
            }

            // Backspace and Delete
            if (evt->is<sf::Event::KeyPressed>())
            {
                if (const auto *kp = evt->getIf<sf::Event::KeyPressed>())
                {
                    auto key = kp->code;
                    if (activeBubble && key == sf::Keyboard::Key::Backspace)
                    {
                        auto t = activeBubble->getText();
                        if (!t.empty())
                        {
                            t.pop_back();
                            activeBubble->setText(t);
                        }
                    }
                    if (key == sf::Keyboard::Key::Delete && picked == PickKind::Bubble && pickedIndex >= 0)
                    {
                        bubbles.erase(bubbles.begin() + pickedIndex);
                        activeBubble = nullptr;
                        picked = PickKind::None;
                        pickedIndex = -1;
                        resizing = false;
                        resizeKind = PickKind::None;
                        resizeIndex = -1;
                        draggingBubble = false;
                        dragBubbleIdx = -1;
                    }
                }
            }
        }
#else
        // ------------------------- SFML 2 events -------------------------
        sf::Event evt;
        while (window.pollEvent(evt))
        {
            if (evt.type == sf::Event::Closed)
                window.close();

            if (evt.type == sf::Event::MouseButtonPressed)
            {
                auto mp = sf::Mouse::getPosition(window);
                sf::Vector2f mpos{static_cast<float>(mp.x), static_cast<float>(mp.y)};

                if (mpos.x <= SidebarW)
                {
                    for (const auto &it : palette)
                    {
                        if (it.hit.contains(mpos.x, mpos.y))
                        { // 2.x contains(x,y)
                            if (it.kind == ItemKind::Hero || it.kind == ItemKind::Villain)
                            {
                                spawnSprite(it.kind, {SidebarW + 24.f, 24.f});
                                activeBubble = nullptr;
                                picked = PickKind::Sprite;
                                pickedIndex = static_cast<int>(sprites.size()) - 1;
                            }
                            else
                            {
                                spawnBubble({SidebarW + 24.f, 24.f});
                                activeBubble = bubbles.empty() ? nullptr : bubbles.back().get();
                                picked = PickKind::Bubble;
                                pickedIndex = static_cast<int>(bubbles.size()) - 1;
                            }
                            break;
                        }
                    }
                    continue;
                }

                picked = PickKind::None;
                pickedIndex = -1;
                activeBubble = nullptr;

                bool hit = false;
                for (int i = static_cast<int>(bubbles.size()) - 1; i >= 0 && !hit; --i)
                {
                    auto r = bubbleRect(*bubbles[i]);
                    if (handleRect(r).contains(mpos.x, mpos.y))
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
                for (int i = static_cast<int>(sprites.size()) - 1; i >= 0 && !hit; --i)
                {
                    auto r = spriteBounds(sprites[i]);
                    if (handleRect(r).contains(mpos.x, mpos.y))
                    {
                        resizing = true;
                        resizeKind = PickKind::Sprite;
                        resizeIndex = i;
                        resizeStartMouse = mpos;
                        resizeStartScale = sprites[i].getScale();
                        resizeStartPos = sprites[i].getPosition();
                        picked = PickKind::Sprite;
                        pickedIndex = i;
                        hit = true;
                    }
                }
                if (hit)
                    continue;

                draggingSprite = false;
                dragSpriteIdx = -1;
                for (int i = static_cast<int>(sprites.size()) - 1; i >= 0; --i)
                {
                    if (sprites[i].getGlobalBounds().contains(mpos.x, mpos.y))
                    {
                        draggingSprite = true;
                        dragSpriteIdx = i;
                        dragOffset = mpos - sprites[i].getPosition();
                        picked = PickKind::Sprite;
                        pickedIndex = i;
                        break;
                    }
                }

                if (!draggingSprite)
                {
                    draggingBubble = false;
                    dragBubbleIdx = -1;
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

            if (evt.type == sf::Event::MouseButtonReleased)
            {
                resizing = false;
                resizeKind = PickKind::None;
                resizeIndex = -1;
                draggingSprite = draggingBubble = false;
                dragSpriteIdx = dragBubbleIdx = -1;
            }

            if (evt.type == sf::Event::MouseMoved)
            {
                sf::Vector2f mpos{static_cast<float>(evt.mouseMove.x), static_cast<float>(evt.mouseMove.y)};

                if (resizing && resizeIndex >= 0)
                {
                    sf::Vector2f delta = mpos - resizeStartMouse;
                    if (resizeKind == PickKind::Bubble)
                    {
                        sf::Vector2f newSize = resizeStartSize + sf::Vector2f{delta.x, delta.y};
                        newSize.x = std::max(newSize.x, 60.f);
                        newSize.y = std::max(newSize.y, 40.f);
                        bubbles[resizeIndex]->setSize(newSize);
                        bubbles[resizeIndex]->setPosition(resizeStartPos);
                    }
                    else if (resizeKind == PickKind::Sprite)
                    {
                        float factor = 1.f + (delta.x / 200.f);
                        factor = std::max(0.25f, std::min(4.f, factor));
                        auto base = resizeStartScale;
                        sprites[resizeIndex].setScale({base.x * factor, base.y * factor});
                        sprites[resizeIndex].setPosition(resizeStartPos);
                    }
                }

                if (draggingSprite && dragSpriteIdx >= 0)
                {
                    auto pos = mpos - dragOffset;
                    sprites[dragSpriteIdx].setPosition(pos);
                }
                if (draggingBubble && dragBubbleIdx >= 0)
                {
                    auto pos = mpos - dragOffset;
                    bubbles[dragBubbleIdx]->setPosition(pos);
                }
            }

            if (evt.type == sf::Event::TextEntered)
            {
                if (activeBubble)
                {
                    char32_t u = static_cast<char32_t>(evt.text.unicode);
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

            if (evt.type == sf::Event::KeyPressed)
            {
                auto key = evt.key.code;
                if (activeBubble && key == sf::Keyboard::Backspace)
                {
                    auto t = activeBubble->getText();
                    if (!t.empty())
                    {
                        t.pop_back();
                        activeBubble->setText(t);
                    }
                }
                if (key == sf::Keyboard::Delete && picked == PickKind::Bubble && pickedIndex >= 0)
                {
                    bubbles.erase(bubbles.begin() + pickedIndex);
                    activeBubble = nullptr;
                    picked = PickKind::None;
                    pickedIndex = -1;
                    resizing = false;
                    resizeKind = PickKind::None;
                    resizeIndex = -1;
                    draggingBubble = false;
                    dragBubbleIdx = -1;
                }
            }
        }
#endif // end cross-version events

        // Render
        window.clear(sf::Color(250, 250, 250));
        window.draw(sidebarBg);

        // Palette previews (no labels)
        {
            auto &AM = AssetManager::getInstance();
            for (const auto &row : palette)
            {
                sf::RectangleShape r;
#if (SFML_VERSION_MAJOR >= 3)
                r.setPosition(row.hit.position);
                r.setSize(row.hit.size);
#else
                r.setPosition({row.hit.left, row.hit.top});
                r.setSize({row.hit.width, row.hit.height});
#endif
                r.setFillColor(sf::Color(245, 245, 245));
                r.setOutlineColor(sf::Color(180, 180, 180));
                r.setOutlineThickness(1.f);
                window.draw(r);

                sf::Vector2f pad{12.f, 10.f};
#if (SFML_VERSION_MAJOR >= 3)
                sf::Vector2f boxTL = row.hit.position + pad;
                sf::Vector2f boxSize = row.hit.size - sf::Vector2f{pad.x * 2.f, pad.y * 2.f};
#else
                sf::Vector2f boxTL = sf::Vector2f{row.hit.left, row.hit.top} + pad;
                sf::Vector2f boxSize = sf::Vector2f{row.hit.width, row.hit.height} - sf::Vector2f{pad.x * 2.f, pad.y * 2.f};
#endif

                switch (row.kind)
                {
                case ItemKind::Hero:
                case ItemKind::Villain:
                {
                    const char *key = (row.kind == ItemKind::Hero) ? "hero_face" : "villain_face";
                    if (auto tex = AM.getTexture(key))
                    {
                        sf::Sprite s(*tex);
                        float sc = std::min(boxSize.x / tex->getSize().x, boxSize.y / tex->getSize().y);
                        sf::Vector2f sprSize{tex->getSize().x * sc, tex->getSize().y * sc};
                        sf::Vector2f pos = boxTL + 0.5f * (boxSize - sprSize);
                        s.setScale({sc, sc});
                        s.setPosition(pos);
                        window.draw(s);
                    }
                    break;
                }
                case ItemKind::Speech:
                {
                    float w = std::max(60.f, boxSize.x);
                    float h = std::max(32.f, boxSize.y);
                    sf::Vector2f pos = boxTL + 0.5f * (boxSize - sf::Vector2f{w, h});
                    SpeechBubble bub("preview", "", pos.x, pos.y, w, h);
                    bub.setStyle("speech");
                    bub.draw(window);
                    break;
                }
                }
            }
        }

        for (const auto &s : sprites)
            window.draw(s);
        for (const auto &b : bubbles)
            b->draw(window);

        auto drawHandle = [&](const sf::FloatRect &r)
        {
#if (SFML_VERSION_MAJOR >= 3)
            auto hr = handleRect(r);
            sf::RectangleShape h;
            h.setPosition(hr.position);
            h.setSize(hr.size);
#else
            auto hr = handleRect(r);
            sf::RectangleShape h;
            h.setPosition({hr.left, hr.top});
            h.setSize({hr.width, hr.height});
#endif
            h.setFillColor(sf::Color(60, 60, 60));
            window.draw(h);
        };
        if (picked == PickKind::Sprite && pickedIndex >= 0)
        {
            drawHandle(spriteBounds(sprites[pickedIndex]));
        }
        else if (picked == PickKind::Bubble && pickedIndex >= 0)
        {
            drawHandle(bubbleRect(*bubbles[pickedIndex]));
        }

        window.display();
    }
    return 0;
}
