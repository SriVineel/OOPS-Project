//=============================================================================
// AssetManager.h
//=============================================================================
// PURPOSE:
//   Centralized resource manager for textures and fonts.
//   Implements singleton pattern for global access.
//   Provides auto-discovery of assets from directories.
//
// KEY FEATURES:
//   - Singleton access via getInstance()
//   - Automatic asset discovery from directories
//   - Shared pointer management for textures (memory efficient)
//   - Asset metadata tracking for UI display
//
// ASSET TYPES:
//   - Textures: Character sprites, bubble images (png files)
//   - Fonts: Text rendering fonts (ttf files)
//
// AUTO-DISCOVERY:
//   Call autoLoadCharacters(), autoLoadFonts(), autoLoadBubbles()
//   to scan directories and load all matching files automatically.
//
// WHERE TO MODIFY:
//   - Add new asset types: Add new map and load/get methods
//   - Change file extensions: Modify autoLoad*() methods
//   - Add asset validation: Extend load methods with size/format checks
//   - Implement unloading: Add clear() or unload() methods
//=============================================================================

#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

// Type alias for shared texture pointers (allows multiple sprites to share same texture)
using TexturePtr = std::shared_ptr<sf::Texture>;

// Asset metadata structure for UI display and queries
struct AssetInfo {
    std::string type;  // "CHARACTER", "FONT", or "BUBBLE"
    std::string key;   // Internal name (filename without extension)
    std::string path;  // Full file path
};

class AssetManager {
private:
    std::map<std::string, TexturePtr> m_textures;  // Texture cache (key -> shared texture)
    std::map<std::string, sf::Font> m_fonts;       // Font cache (key -> font object)
    std::vector<AssetInfo> m_assetList;            // All loaded assets metadata

    // Private constructor for singleton pattern
    AssetManager() = default;

public:
    //-------------------------------------------------------------------------
    // SINGLETON ACCESS - Use this to get the single instance
    //-------------------------------------------------------------------------

    // Get the global AssetManager instance
    // Usage: AssetManager::getInstance().loadTexture(...)
    static AssetManager& getInstance();

    // Prevent copying and assignment (singleton pattern)
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    //-------------------------------------------------------------------------
    // MANUAL LOADING - Load individual assets by name and path
    //-------------------------------------------------------------------------

    // Load texture from file and store with given name
    // Throws runtime_error if file cannot be loaded
    void loadTexture(const std::string& name, const std::string& filename);

    // Get texture by name (returns nullptr if not found)
    TexturePtr getTexture(const std::string& name) const;

    // Load font from file and store with given name
    // Throws runtime_error if file cannot be loaded
    void loadFont(const std::string& name, const std::string& filename);

    // Get font by name (throws runtime_error if not found)
    sf::Font& getFont(const std::string& name);

    //-------------------------------------------------------------------------
    // AUTO-DISCOVERY - Automatically load all assets from directories
    //-------------------------------------------------------------------------

    // Scan directory for character images (png files)
    // Automatically names them by filename without extension
    void autoLoadCharacters(const std::string& dir);

    // Scan directory for font files (ttf files)
    void autoLoadFonts(const std::string& dir);

    // Scan directory for bubble background images (png files)
    void autoLoadBubbles(const std::string& dir);

    //-------------------------------------------------------------------------
    // QUERY METHODS - Get information about loaded assets
    //-------------------------------------------------------------------------

    // Get list of all loaded assets with metadata
    const std::vector<AssetInfo>& getAssetList() const;

    // Get all asset keys of a specific type
    // type should be "CHARACTER", "FONT", or "BUBBLE"
    std::vector<std::string> getAssetsByType(const std::string& type) const;
};