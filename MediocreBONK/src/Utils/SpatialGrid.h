#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include "../ECS/Entity.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Collider.h"

namespace MediocreBONK::Utils
{
    /*
     * OPTIMIZATION TECHNIQUE: SPATIAL PARTITIONING (Grid-based)
     *
     * Problem:
     * - Naive collision detection: O(n²) comparisons
     * - With 1000 entities: 500,000 collision checks per frame!
     * - Most checks are wasted (entities far apart)
     *
     * Solution: Spatial Grid
     * - Divide world into grid cells (e.g., 100x100 pixels each)
     * - Each entity stored in cells it overlaps
     * - Query only checks entities in nearby cells
     * - Reduces checks from O(n²) to O(n) in typical cases
     *
     * How it works:
     * 1. Each frame: insert() all entities into grid cells
     * 2. For each entity: query() nearby cells for potential colliders
     * 3. Only test collision against entities in nearby cells
     *
     * Example:
     * - World with 1000 entities
     * - Without grid: 500,000 collision checks
     * - With grid: ~50 checks per entity = 50,000 total (10x faster!)
     *
     * Trade-offs:
     * - Memory: Hash map stores cell -> entity list
     * - Rebuild cost: Must insert all entities each frame
     * - Grid too small: entities span many cells (overhead)
     * - Grid too large: cells contain too many entities (less effective)
     *
     * Optimal cell size:
     * - Should be ~2x the size of typical entity
     * - For this game: 100 pixels (entities are 10-40 pixels)
     *
     * Alternative approaches:
     * - Quadtree: Better for non-uniform entity distribution
     * - Spatial hash: Similar but uses hash function instead of grid
     * - Bounding volume hierarchy: Better for static geometry
     */
    class SpatialGrid
    {
    public:
        SpatialGrid(float cellSize = 100.f)
            : cellSize(cellSize)
        {}

        // Clear grid (called at start of each frame)
        void clear()
        {
            grid.clear();
        }

        // Insert entity into all grid cells it overlaps
        // Large entities may span multiple cells
        void insert(ECS::Entity* entity)
        {
            auto* transform = entity->getComponent<ECS::Components::Transform>();
            auto* collider = entity->getComponent<ECS::Components::Collider>();

            if (!transform || !collider) return;

            // Calculate which grid cells this entity overlaps
            // Entity's bounding box: (position - radius) to (position + radius)
            int minX = static_cast<int>((transform->position.x - collider->radius) / cellSize);
            int maxX = static_cast<int>((transform->position.x + collider->radius) / cellSize);
            int minY = static_cast<int>((transform->position.y - collider->radius) / cellSize);
            int maxY = static_cast<int>((transform->position.y + collider->radius) / cellSize);

            // Insert entity into all overlapping cells
            for (int x = minX; x <= maxX; ++x)
            {
                for (int y = minY; y <= maxY; ++y)
                {
                    grid[getKey(x, y)].push_back(entity);
                }
            }
        }

        // SPATIAL QUERY: Find all entities near a position
        // Returns: List of entities in nearby grid cells
        // Note: May contain duplicates if entity spans multiple cells
        std::vector<ECS::Entity*> query(const sf::Vector2f& position, float radius)
        {
            std::vector<ECS::Entity*> result;

            // Calculate which cells to search
            // Search area: circle of given radius around position
            int minX = static_cast<int>((position.x - radius) / cellSize);
            int maxX = static_cast<int>((position.x + radius) / cellSize);
            int minY = static_cast<int>((position.y - radius) / cellSize);
            int maxY = static_cast<int>((position.y + radius) / cellSize);

            // OPTIMIZATION: Could return duplicates for speed (let caller handle)
            // QUALITY: Removing duplicates here for correctness

            // Collect entities from all nearby cells
            for (int x = minX; x <= maxX; ++x)
            {
                for (int y = minY; y <= maxY; ++y)
                {
                    auto it = grid.find(getKey(x, y));
                    if (it != grid.end())
                    {
                        // Add all entities in this cell
                        result.insert(result.end(), it->second.begin(), it->second.end());
                    }
                }
            }

            // Remove duplicates (entity may be in multiple cells)
            // Sort + unique is faster than set for small lists
            std::sort(result.begin(), result.end());
            result.erase(std::unique(result.begin(), result.end()), result.end());

            return result;
        }

    private:
        // Hash 2D coordinates to single key for unordered_map
        // Combines x and y into 64-bit integer
        // Upper 32 bits = x, lower 32 bits = y
        long long getKey(int x, int y)
        {
            return (static_cast<long long>(x) << 32) | (static_cast<unsigned int>(y));
        }

        // Grid parameters
        float cellSize; // Size of each grid cell in world units

        // Grid storage: cell key -> list of entities in that cell
        // Rebuilt each frame (entities move, so grid changes)
        std::unordered_map<long long, std::vector<ECS::Entity*>> grid;
    };
}
