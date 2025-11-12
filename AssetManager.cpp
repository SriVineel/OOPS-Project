// AssetManager.cpp
// Implements the process-wide asset cache with simple error handling.
// Design decisions:
// - Textures are shared_ptr so sprites can share GPU resources safely.
// - Fonts are stored by value because sf::Text holds a non-owning pointer to a font.

#include "AssetManager.h"

#include <stdexcept> // std::runtime_error
#include <utility>   // std::move

AssetManager& AssetManager::getInstance() {
    static AssetManager instance; // Meyers' singleton: thread-safe init in modern C++
    return instance;
}

void AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    auto tex = std::make_shared<sf::Texture>();
    if (!tex->loadFromFile(filename)) {
        throw std::runtime_error("Texture load failed: " + filename);
    }
    m_textures[name] = std::move(tex);
}

TexturePtr AssetManager::getTexture(const std::string& name) const {
    auto it = m_textures.find(name);
    return it == m_textures.end() ? nullptr : it->second;
}

void AssetManager::loadFont(const std::string& name, const std::string& filename) {
    sf::Font font;
    if (!font.openFromFile(filename)) { // SFML 3 API
        throw std::runtime_error("Font load failed: " + filename);
    }
    m_fonts[name] = std::move(font);
}

sf::Font& AssetManager::getFont(const std::string& name) {
    // at() throws std::out_of_range if missing; this is intentional to fail fast.
    return m_fonts.at(name);
}
