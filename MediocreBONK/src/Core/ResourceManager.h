#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <memory>
#include <cassert>
#include "../Utils/Logger.h"

namespace MediocreBONK::Core
{
    /*
     * DESIGN PATTERN: SINGLETON PATTERN (Meyer's Singleton)
     *
     * Purpose:
     * - Ensures only ONE instance of ResourceManager exists throughout the program
     * - Provides global access point to shared resources
     * - Prevents duplicate loading of expensive resources (textures, fonts, sounds)
     *
     * Implementation:
     * - Private constructor prevents direct instantiation
     * - Static getInstance() creates instance on first call (lazy initialization)
     * - Deleted copy constructor/assignment prevents duplication
     * - Thread-safe in C++11+ (static local variables are initialized once)
     *
     * Benefits:
     * - Memory efficient: resources loaded once and shared
     * - Consistent state: all systems access same resource cache
     * - Automatic lifetime management: destroyed when program ends
     *
     * Trade-offs:
     * - Global state can make testing harder
     * - Tight coupling between systems and ResourceManager
     * - Can hide dependencies
     *
     * DESIGN PATTERN: RESOURCE CACHING / FLYWEIGHT PATTERN
     *
     * Purpose:
     * - Cache loaded resources to avoid redundant disk I/O
     * - Share heavy objects (textures, fonts) across multiple entities
     *
     * Implementation:
     * - std::unordered_map stores resources by filename
     * - getTexture() checks cache before loading from disk
     * - Returns const reference to cached resource (no copying)
     */
    class ResourceManager
    {
    public:
        // SINGLETON PATTERN: Global access point
        // Meyer's singleton: thread-safe, lazy initialization, automatic destruction
        static ResourceManager& getInstance()
        {
            static ResourceManager instance; // Created once, destroyed automatically
            return instance;
        }

        // SINGLETON PATTERN: Prevent copying
        // Delete copy constructor and assignment operator to ensure single instance
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        // CACHING PATTERN: Load once, return cached reference on subsequent calls
        // This prevents loading the same texture multiple times from disk
        const sf::Texture& getTexture(const std::string& filename)
        {
            // Step 1: Check cache first (O(1) lookup via hash map)
            auto it = textures.find(filename);
            if (it != textures.end())
            {
                return it->second; // Return cached texture
            }

            // Step 2: Cache miss - load from disk (expensive I/O operation)
            sf::Texture texture;
            if (!texture.loadFromFile(filename))
            {
                assert(false && "Failed to load texture");
            }

            // Step 3: Store in cache using std::move (avoids copying texture data)
            textures[filename] = std::move(texture);
            Utils::Logger::info("Loaded texture: " + filename);
            return textures[filename]; // Return newly cached texture
        }

        // Load and get font
        const sf::Font& getFont(const std::string& filename)
        {
            // Check if already loaded
            auto it = fonts.find(filename);
            if (it != fonts.end())
            {
                return it->second;
            }

            // Load font using SFML 3 API
            sf::Font font;
            if (!font.openFromFile(filename))
            {
                assert(false && "Failed to load font");
            }

            fonts[filename] = std::move(font);
            Utils::Logger::info("Loaded font: " + filename);
            return fonts[filename];
        }

        // Load and get sound buffer
        const sf::SoundBuffer& getSoundBuffer(const std::string& filename)
        {
            // Check if already loaded
            auto it = soundBuffers.find(filename);
            if (it != soundBuffers.end())
            {
                return it->second;
            }

            // Load sound buffer using SFML 3 API
            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile(filename))
            {
                assert(false && "Failed to load sound buffer");
            }

            soundBuffers[filename] = std::move(buffer);
            Utils::Logger::info("Loaded sound: " + filename);
            return soundBuffers[filename];
        }

        // Clear all resources
        void clear()
        {
            textures.clear();
            fonts.clear();
            soundBuffers.clear();
            Utils::Logger::info("Cleared all resources");
        }

    private:
        // SINGLETON PATTERN: Private constructor prevents direct instantiation
        // Only getInstance() can create the single instance
        ResourceManager() = default;

        // CACHING: Hash maps provide O(1) lookup for cached resources
        // Key = filename, Value = loaded resource
        std::unordered_map<std::string, sf::Texture> textures;
        std::unordered_map<std::string, sf::Font> fonts;
        std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    };
}
