//=============================================================================
// AssetManager.cpp
//=============================================================================
// PURPOSE:
//   Implementation of the AssetManager singleton. Loads and provides access
//   to textures and fonts used across the application.
//
// KEY FEATURES:
//   - Manual loading and auto-discovery of assets from directories
//   - Texture and font caching to avoid duplicate loads
//   - Simple API: loadTexture/getTexture, loadFont/getFont, autoLoad*
//
// WHERE TO MODIFY:
//   - Change supported file extensions in autoLoad* functions
//   - Add validation or pre-processing of assets during load
//   - Implement asset unloading or reference counting if required
//=============================================================================

#include "AssetManager.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
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
    if (!font.openFromFile(filename)) {
        throw std::runtime_error("Font load failed: " + filename);
    }
    m_fonts[name] = std::move(font);
}

sf::Font& AssetManager::getFont(const std::string& name) {
    auto it = m_fonts.find(name);
    if (it == m_fonts.end()) {
        throw std::runtime_error("Font not found: " + name);
    }
    return it->second;
}

// Auto-load all character images from directory (case-insensitive extensions)
void AssetManager::autoLoadCharacters(const std::string& dir) {
    if (!fs::exists(dir)) {
        std::cerr << "[AssetManager] Directory not found: " << dir << "\n";
        return;
    }

    std::cout << "[AssetManager] Scanning characters in: " << dir << "\n";
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();
        std::string ext  = entry.path().extension().string();
        std::string name = entry.path().stem().string();

        // make extension lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
            try {
                loadTexture(name, path);
                m_assetList.push_back({ "CHARACTER", name, path });
                std::cout << "  [✓] Loaded character: " << name << "\n";
            } catch (const std::exception& e) {
                std::cerr << "  [x] Failed: " << name << " - " << e.what() << "\n";
            }
        }
    }
}


// Auto-load all fonts from directory (case-insensitive extensions)
void AssetManager::autoLoadFonts(const std::string& dir) {
    if (!fs::exists(dir)) {
        std::cerr << "[AssetManager] Directory not found: " << dir << "\n";
        return;
    }

    std::cout << "[AssetManager] Scanning fonts in: " << dir << "\n";
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();
        std::string ext  = entry.path().extension().string();
        std::string name = entry.path().stem().string();

        // make extension lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (ext == ".ttf" || ext == ".otf") {
            try {
                loadFont(name, path);
                m_assetList.push_back({ "FONT", name, path });
                std::cout << "  [✓] Loaded font: " << name << "\n";
            } catch (const std::exception& e) {
                std::cerr << "  [x] Failed: " << name << " - " << e.what() << "\n";
            }
        }
    }
}



// Auto-load all speech bubble images from directory (case-insensitive extensions)
void AssetManager::autoLoadBubbles(const std::string& dir) {
    if (!fs::exists(dir)) {
        std::cerr << "[AssetManager] Directory not found: " << dir << "\n";
        return;
    }

    std::cout << "[AssetManager] Scanning bubbles in: " << dir << "\n";
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();
        std::string ext  = entry.path().extension().string();
        std::string name = entry.path().stem().string();

        // make extension lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
            try {
                // Prefix with "bubble_" to match SpeechBubble::setStyle() expectations
                std::string key = "bubble_" + name;
                loadTexture(key, path);
                m_assetList.push_back({ "BUBBLE", name, path });
                std::cout << "  [✓] Loaded bubble: " << name
                          << " (key: " << key << ")\n";
            } catch (const std::exception& e) {
                std::cerr << "  [x] Failed: " << name << " - " << e.what() << "\n";
            }
        }
    }
}


// NEW: Filter assets by type
std::vector<std::string> AssetManager::getAssetsByType(const std::string& type) const {
    std::vector<std::string> result;
    
    for (const auto& info : m_assetList) {
        if (info.type == type) {
            result.push_back(info.key);  // Push only the key string, not the whole object
        }
    }
    
    return result;
}

