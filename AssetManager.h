// AssetManager.h
// Purpose: Centralized asset cache for textures and fonts used across the app.
// - Avoids duplicate loads from disk and centralizes error handling.
// - Textures are stored as shared_ptr to share ownership with SFML sprites safely.
// - Fonts are stored by value and returned as references (they must outlive any sf::Text using them).
// Notes (SFML 3):
// - Load texture via sf::Texture::loadFromFile.
// - Load font via sf::Font::openFromFile.
// Thread-safety: This simple singleton is not thread-safe; use only from the main/UI thread.

#pragma once

#include <memory>   // std::shared_ptr
#include <map>      // std::map
#include <string>   // std::string
#include <SFML/Graphics.hpp> // sf::Texture, sf::Font

// Shareable handle for textures used by sf::Sprite; allows lightweight copies of sprites.
using TexturePtr = std::shared_ptr<sf::Texture>;

class AssetManager {
public:
    // Load a texture from disk and store it under a unique logical name.
    // Throws std::runtime_error if the file cannot be loaded.
    void loadTexture(const std::string& name, const std::string& filename);

    // Retrieve a previously loaded texture; returns nullptr if not found.
    // Callers should check for nullptr before using.
    TexturePtr getTexture(const std::string& name) const;

    // Load a font from disk and store it under a logical name.
    // Throws std::runtime_error if the file cannot be opened.
    void loadFont(const std::string& name, const std::string& filename);

    // Retrieve a previously loaded font by reference.
    // Throws std::out_of_range if the font name is not present.
    sf::Font& getFont(const std::string& name);

    // Access the process-wide singleton instance.
    static AssetManager& getInstance();

private:
    // Texture storage (shared ownership for easy sharing across sprites).
    std::map<std::string, TexturePtr> m_textures;

    // Font storage (by value; returned by non-owning reference).
    std::map<std::string, sf::Font> m_fonts;

    // Singleton boilerplate
    AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};
