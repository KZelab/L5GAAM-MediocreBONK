#pragma once
#include "../ECS/Components/Transform.h"
#include "../Utils/Logger.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <cmath>

namespace MediocreBONK::Systems
{
    struct TileKey
    {
        int x;
        int y;

        bool operator==(const TileKey& other) const
        {
            return x == other.x && y == other.y;
        }
    };

    struct TileKeyHash
    {
        std::size_t operator()(const TileKey& key) const
        {
            // Combine hash of x and y
            return std::hash<int>()(key.x) ^ (std::hash<int>()(key.y) << 1);
        }
    };

    class WorldGenerator
    {
    public:
        WorldGenerator(float tileSize = 1000.f)
            : tileSize(tileSize)
            , renderDistance(2) // Render tiles 2 away from player
        {
            Utils::Logger::info("WorldGenerator initialized");
        }

        void update(const sf::Vector2f& playerPosition)
        {
            // Calculate which tile the player is currently in
            int playerTileX = static_cast<int>(std::floor(playerPosition.x / tileSize));
            int playerTileY = static_cast<int>(std::floor(playerPosition.y / tileSize));

            // Generate tiles around the player
            for (int x = playerTileX - renderDistance; x <= playerTileX + renderDistance; ++x)
            {
                for (int y = playerTileY - renderDistance; y <= playerTileY + renderDistance; ++y)
                {
                    TileKey key{ x, y };
                    if (activeTiles.find(key) == activeTiles.end())
                    {
                        // Generate new tile
                        generateTile(key);
                    }
                }
            }

            // Remove tiles that are too far from player
            std::vector<TileKey> tilesToRemove;
            for (const auto& [key, tile] : activeTiles)
            {
                int distX = std::abs(key.x - playerTileX);
                int distY = std::abs(key.y - playerTileY);

                if (distX > renderDistance + 1 || distY > renderDistance + 1)
                {
                    tilesToRemove.push_back(key);
                }
            }

            for (const auto& key : tilesToRemove)
            {
                activeTiles.erase(key);
            }
        }

        void render(sf::RenderWindow& window)
        {
            for (const auto& [key, tile] : activeTiles)
            {
                window.draw(tile);
            }
        }

        float getTileSize() const { return tileSize; }

    private:
        void generateTile(const TileKey& key)
        {
            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
            tile.setPosition({key.x * tileSize, key.y * tileSize});

            // Generate procedural color based on tile coordinates
            // This creates a checkerboard-like pattern
            bool isDark = (key.x + key.y) % 2 == 0;
            if (isDark)
            {
                tile.setFillColor(sf::Color(40, 40, 60)); // Dark blue-grey
            }
            else
            {
                tile.setFillColor(sf::Color(50, 50, 70)); // Slightly lighter
            }

            // Add grid lines for visual clarity
            tile.setOutlineThickness(2.f);
            tile.setOutlineColor(sf::Color(30, 30, 50, 100)); // Subtle outline

            activeTiles[key] = tile;
        }

        float tileSize;
        int renderDistance;
        std::unordered_map<TileKey, sf::RectangleShape, TileKeyHash> activeTiles;
    };
}
