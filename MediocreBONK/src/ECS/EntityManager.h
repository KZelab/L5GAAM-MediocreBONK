#pragma once
#include "Entity.h"
#include "../Utils/Logger.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace MediocreBONK::ECS
{
    /*
     * DESIGN PATTERN: ENTITY-COMPONENT-SYSTEM (ECS) + OBJECT POOLING
     *
     * EntityManager (this class):
     * - Factory for creating/destroying entities
     * - Central repository of all entities
     * - Query interface for finding entities by tag/layer/components
     *
     * OBJECT POOLING PATTERN:
     * - Instead of delete/new, entities are deactivated/reactivated
     * - Inactive entities stay in memory (marked inactive)
     * - createEntity() first tries to reuse inactive entity
     * - Periodically cleans up inactive entities to free memory
     *
     * Object Pooling Benefits:
     * - Reduces memory allocations (expensive in game loops)
     * - Eliminates heap fragmentation
     * - More cache-friendly (entities stored contiguously)
     * - Prevents allocation spikes (e.g., spawning 100 enemies at once)
     *
     * Trade-offs:
     * - Memory usage higher (inactive entities stay allocated)
     * - Requires periodic cleanup to prevent memory growth
     * - Must be careful with stale pointers after cleanup
     *
     * Example Usage:
     *   auto* entity = entityManager->createEntity();
     *   entity->tag = "Enemy";
     *   entity->addComponent<Transform>(100.f, 200.f);
     *
     *   // Later:
     *   entityManager->destroyEntity(entity); // Marks inactive (pooled)
     *
     * Query Interface:
     * - getEntitiesByTag("Enemy"): Returns all enemies
     * - getEntitiesWithComponent<Health>(): Returns all entities with Health
     * - getEntitiesWithComponents<Transform, Health>(): Multiple components
     */
    class EntityManager
    {
    public:
        EntityManager(size_t maxEntities = 500)
            : maxEntities(maxEntities)
            , nextId(0)
            , cleanupTimer(0.f)
            , cleanupInterval(0.5f) // Cleanup every 0.5 seconds
            , tagCacheDirty(true) // Start dirty to build cache on first update
        {
            entities.reserve(maxEntities); // Pre-allocate vector capacity
            Utils::Logger::info("EntityManager initialized with max entities: " + std::to_string(maxEntities));
        }

        ~EntityManager()
        {
            clear();
        }

        // OBJECT POOLING: Create new entity or reuse inactive one
        Entity* createEntity()
        {
            // Check if we've hit the entity cap (prevents unbounded growth)
            if (entities.size() >= maxEntities)
            {
                Utils::Logger::warning("Entity cap reached! Attempting to reuse inactive entity.");

                // OBJECT POOLING: Try to reuse an inactive entity
                auto it = std::find_if(entities.begin(), entities.end(),
                    [](const std::unique_ptr<Entity>& e) { return !e->isActive(); });

                if (it != entities.end())
                {
                    (*it)->setActive(true); // Reactivate pooled entity
                    Utils::Logger::info("Reused entity ID: " + std::to_string((*it)->getId()));
                    return it->get();
                }

                // Pool exhausted: cap reached and all entities active
                Utils::Logger::error("Cannot create entity - cap reached and no inactive entities available!");
                return nullptr;
            }

            // Create new entity (below cap)
            auto entity = std::make_unique<Entity>(nextId++);
            Entity* ptr = entity.get();
            entities.push_back(std::move(entity));

            // OPTIMIZATION: Mark cache dirty since we added an entity
            tagCacheDirty = true;

            return ptr;
        }

        // Get entity by ID
        Entity* getEntity(uint64_t id)
        {
            auto it = std::find_if(entities.begin(), entities.end(),
                [id](const std::unique_ptr<Entity>& e) { return e->getId() == id; });

            if (it != entities.end())
            {
                return it->get();
            }
            return nullptr;
        }

        // OBJECT POOLING: Destroy entity (actually just deactivate it)
        // Entity stays in memory for potential reuse
        void destroyEntity(uint64_t id)
        {
            auto entity = getEntity(id);
            if (entity)
            {
                entity->setActive(false); // Mark inactive (not deleted!)
                // OPTIMIZATION: Mark cache dirty since we deactivated an entity
                tagCacheDirty = true;
            }
        }

        // OBJECT POOLING: Destroy entity by pointer
        void destroyEntity(Entity* entity)
        {
            if (entity)
            {
                entity->setActive(false); // Mark inactive (pooled)
                // OPTIMIZATION: Mark cache dirty since we deactivated an entity
                tagCacheDirty = true;
            }
        }

        // Get all entities with a specific tag
        const std::vector<Entity*>& getEntitiesByTag(const std::string& tag)
        {
            static const std::vector<Entity*> empty;
            auto it = tagCache.find(tag);
            if (it != tagCache.end())
            {
                return it->second;
            }
            return empty;
        }

        // Get all entities on a specific layer
        std::vector<Entity*> getEntitiesByLayer(uint32_t layer)
        {
            std::vector<Entity*> result;
            for (auto& entity : entities)
            {
                if (entity->isActive() && entity->layer == layer)
                {
                    result.push_back(entity.get());
                }
            }
            return result;
        }

        // Get all active entities
        std::vector<Entity*> getActiveEntities()
        {
            std::vector<Entity*> result;
            for (auto& entity : entities)
            {
                if (entity->isActive())
                {
                    result.push_back(entity.get());
                }
            }
            return result;
        }

        // ECS QUERY: Get entities with specific component type
        // Used by systems to process entities (e.g., CollisionSystem queries all Collider entities)
        template<typename T>
        std::vector<Entity*> getEntitiesWithComponent()
        {
            std::vector<Entity*> result;
            for (auto& entity : entities)
            {
                if (entity->isActive() && entity->hasComponent<T>())
                {
                    result.push_back(entity.get());
                }
            }
            return result;
        }

        // ECS QUERY: Get entities with multiple components (variadic template)
        // Example: getEntitiesWithComponents<Transform, Health, Sprite>()
        // Returns only entities that have ALL specified components
        template<typename T1, typename T2, typename... Rest>
        std::vector<Entity*> getEntitiesWithComponents()
        {
            std::vector<Entity*> result;
            for (auto& entity : entities)
            {
                if (entity->isActive() &&
                    entity->hasComponent<T1>() &&
                    entity->hasComponent<T2>() &&
                    (entity->hasComponent<Rest>() && ...)) // Fold expression (C++17)
                {
                    result.push_back(entity.get());
                }
            }
            return result;
        }

        // Update all active entities
        void update(sf::Time dt)
        {
            // OBJECT POOLING: Periodically clean up inactive entities
            // This prevents the pool from growing unbounded
            // MUST be done BEFORE rebuilding cache to avoid dangling pointers
            cleanupTimer += dt.asSeconds();
            if (cleanupTimer >= cleanupInterval)
            {
                cleanupInactiveEntities();
                cleanupTimer = 0.f;
            }

            // OPTIMIZATION: Only rebuild tag cache when dirty (entities created/destroyed)
            // This avoids rebuilding cache every frame (expensive with many entities)
            if (tagCacheDirty)
            {
                tagCache.clear();
                for (auto& entity : entities)
                {
                    if (entity->isActive())
                    {
                        if (!entity->tag.empty())
                        {
                            tagCache[entity->tag].push_back(entity.get());
                        }
                    }
                }
                tagCacheDirty = false;
            }

            // Update all active entities
            for (auto& entity : entities)
            {
                if (entity->isActive())
                {
                    entity->update(dt);
                }
            }
        }

        // OBJECT POOLING: Actually remove inactive entities from memory
        // This is the cleanup phase that frees memory
        void cleanupInactiveEntities()
        {
            size_t beforeCount = entities.size();
            size_t inactiveCount = beforeCount - getEntityCount();

            // Threshold: Only cleanup if we have enough inactive entities
            // Avoids frequent reallocations (expensive)
            if (inactiveCount < 10)
                return;

            // Erase-remove idiom: remove inactive entities from vector
            entities.erase(
                std::remove_if(entities.begin(), entities.end(),
                    [](const std::unique_ptr<Entity>& e) { return !e->isActive(); }),
                entities.end()
            );

            size_t afterCount = entities.size();
            Utils::Logger::info("Cleaned up " + std::to_string(beforeCount - afterCount) +
                               " inactive entities. Active: " + std::to_string(getEntityCount()) +
                               " Total: " + std::to_string(afterCount));

            // OPTIMIZATION: Mark cache dirty after cleanup
            if (beforeCount != afterCount)
            {
                tagCacheDirty = true;
            }
        }

        // Render all active entities
        void render(sf::RenderWindow& window)
        {
            for (auto& entity : entities)
            {
                if (entity->isActive())
                {
                    entity->render(window);
                }
            }
        }

        // Clear all entities
        void clear()
        {
            entities.clear();
            nextId = 0;
            Utils::Logger::info("EntityManager cleared");
        }

        // Get entity count
        size_t getEntityCount() const
        {
            return std::count_if(entities.begin(), entities.end(),
                [](const std::unique_ptr<Entity>& e) { return e->isActive(); });
        }

        size_t getTotalEntityCount() const
        {
            return entities.size();
        }

    private:
        // Entity storage: vector of unique_ptr for contiguous memory (cache-friendly)
        std::vector<std::unique_ptr<Entity>> entities;

        // OBJECT POOLING: Entity cap prevents unbounded growth
        size_t maxEntities;

        // ID generator: increments for each new entity
        uint64_t nextId;

        // OBJECT POOLING: Cleanup timing
        float cleanupTimer;
        float cleanupInterval;

        // CACHING: Tag-to-entities map for fast tag queries
        // Rebuilt only when dirty flag is set (entities created/destroyed)
        std::unordered_map<std::string, std::vector<Entity*>> tagCache;
        bool tagCacheDirty;
    };
}
