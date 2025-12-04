#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/Health.h"
#include "../ECS/Components/Projectile.h"
#include "../Utils/Math.h"
#include "../Utils/Math.h"
#include "../Utils/Logger.h"
#include "../Utils/SpatialGrid.h"
#include "../Utils/Profiler.h"
#include <SFML/System/Time.hpp>
#include <vector>
#include <algorithm>

namespace MediocreBONK::Systems
{
    class CollisionSystem
    {
    public:
        CollisionSystem(ECS::EntityManager* entityManager)
            : entityManager(entityManager)
            , playerDamageCooldown(0.f)
            , playerDamageInterval(0.5f) // Player can take damage every 0.5 seconds
            , cullingRange(1400.f) // Collision check range (must exceed spawn radius ~1151px)
            , grid(100.f) // Cell size 100
        {}

        void update(sf::Time dt)
        {
            // Update damage cooldown
            if (playerDamageCooldown > 0.f)
                playerDamageCooldown -= dt.asSeconds();

            // Get all entities with colliders
            auto colliders = entityManager->getEntitiesWithComponents<
                ECS::Components::Transform,
                ECS::Components::Collider
            >();

            // Rebuild grid
            Utils::Profiler::start("Grid Rebuild");
            grid.clear();
            for (auto* entity : colliders)
            {
                grid.insert(entity);
            }
            Utils::Profiler::stop("Grid Rebuild");

            // Optimized collision detection using grid
            Utils::Profiler::start("Grid Collision");
            for (auto* entityA : colliders)
            {
                auto* transformA = entityA->getComponent<ECS::Components::Transform>();
                auto* colliderA = entityA->getComponent<ECS::Components::Collider>();

                // Query nearby entities
                // Search radius = collider radius + max expected other radius (e.g. 50)
                float searchRadius = colliderA->radius + 50.f; 
                auto nearby = grid.query(transformA->position, searchRadius);

                for (auto* entityB : nearby)
                {
                    if (entityA == entityB) continue;
                    
                    // Avoid double checking
                    if (entityA->getId() > entityB->getId()) continue;

                    checkCollision(entityA, entityB);
                }
            }
            Utils::Profiler::stop("Grid Collision");

            // Handle projectile-entity collisions separately
            handleProjectileCollisions();

            // Handle player-enemy collisions (damage)
            handlePlayerEnemyCollisions();

            // Handle enemy-enemy separation
            handleEnemySeparation();
        }

    private:
        void checkCollision(ECS::Entity* a, ECS::Entity* b)
        {
            auto* transformA = a->getComponent<ECS::Components::Transform>();
            auto* transformB = b->getComponent<ECS::Components::Transform>();
            auto* colliderA = a->getComponent<ECS::Components::Collider>();
            auto* colliderB = b->getComponent<ECS::Components::Collider>();

            if (colliderA->intersects(colliderB, transformA->position, transformB->position))
            {
                // Trigger callbacks if they exist
                if (colliderA->onCollisionEnter)
                    colliderA->onCollisionEnter(b);

                if (colliderB->onCollisionEnter)
                    colliderB->onCollisionEnter(a);
            }
        }

        void handleProjectileCollisions()
        {
            auto projectiles = entityManager->getEntitiesWithComponents<
                ECS::Components::Transform,
                ECS::Components::Projectile,
                ECS::Components::Collider
            >();

            // Get player position for culling
            auto players = entityManager->getEntitiesByTag("Player");
            sf::Vector2f playerPos(0.f, 0.f);
            if (!players.empty())
            {
                auto* pTransform = players[0]->getComponent<ECS::Components::Transform>();
                if (pTransform)
                    playerPos = pTransform->position;
            }

            Utils::Profiler::start("Proj Collision");
            for (auto* projectile : projectiles)
            {
                auto* projComp = projectile->getComponent<ECS::Components::Projectile>();
                auto* projTransform = projectile->getComponent<ECS::Components::Transform>();
                auto* projCollider = projectile->getComponent<ECS::Components::Collider>();

                // Cull projectiles too far from player
                if (Utils::Math::distanceSquared(projTransform->position, playerPos) > cullingRange * cullingRange)
                    continue;

                // Get potential targets from grid
                std::string targetTag = (projComp->getOwnerTag() == "Player") ? "Enemy" : "Player";
                
                // Query grid
                float searchRadius = projCollider->radius + 50.f; // Assume max enemy radius 50
                auto potentialTargets = grid.query(projTransform->position, searchRadius);

                for (auto* target : potentialTargets)
                {
                    if (target->tag != targetTag) continue;

                    // Skip if already hit this target
                    if (!projComp->canHit(target->getId()))
                        continue;

                    auto* targetTransform = target->getComponent<ECS::Components::Transform>();
                    auto* targetCollider = target->getComponent<ECS::Components::Collider>();
                    auto* targetHealth = target->getComponent<ECS::Components::Health>();

                    if (!targetTransform || !targetCollider || !targetHealth)
                        continue;

                    // Check collision
                    if (projCollider->intersects(targetCollider, projTransform->position, targetTransform->position))
                    {
                        // Apply damage
                        targetHealth->takeDamage(projComp->getDamage());

                        // Record hit
                        projComp->recordHit(target->getId());

                        // Projectile might be deactivated by recordHit if piercing runs out
                        if (!projectile->isActive())
                            break;
                    }
                }
            }
            Utils::Profiler::stop("Proj Collision");
        }



        void handlePlayerEnemyCollisions()
        {
            // Only apply damage if cooldown has expired
            if (playerDamageCooldown > 0.f)
                return;

            auto players = entityManager->getEntitiesByTag("Player");
            auto enemies = entityManager->getEntitiesByTag("Enemy");

            for (auto* player : players)
            {
                auto* playerTransform = player->getComponent<ECS::Components::Transform>();
                auto* playerCollider = player->getComponent<ECS::Components::Collider>();
                auto* playerHealth = player->getComponent<ECS::Components::Health>();

                if (!playerTransform || !playerCollider || !playerHealth)
                    continue;

                for (auto* enemy : enemies)
                {
                    auto* enemyTransform = enemy->getComponent<ECS::Components::Transform>();
                    auto* enemyCollider = enemy->getComponent<ECS::Components::Collider>();

                    if (!enemyTransform || !enemyCollider)
                        continue;

                    // Check collision
                    if (playerCollider->intersects(enemyCollider, playerTransform->position, enemyTransform->position))
                    {
                        // Apply damage to player
                        float enemyDamage = 5.f; // Default damage
                        playerHealth->takeDamage(enemyDamage);

                        // Reset cooldown so we don't apply damage again immediately
                        playerDamageCooldown = playerDamageInterval;

                        // Only apply damage once per cooldown cycle, then return
                        return;
                    }
                }
            }
        }

        void handleEnemySeparation()
        {
            auto enemies = entityManager->getEntitiesByTag("Enemy");

            // Enemy separation - push enemies apart if overlapping
            Utils::Profiler::start("Enemy Separation");
            for (size_t i = 0; i < enemies.size(); ++i)
            {
                auto* enemyA = enemies[i];
                auto* transformA = enemyA->getComponent<ECS::Components::Transform>();
                auto* colliderA = enemyA->getComponent<ECS::Components::Collider>();
                
                if (!transformA || !colliderA) continue;

                // Query grid for neighbors
                float searchRadius = colliderA->radius * 2.5f; // Look for overlapping enemies
                auto neighbors = grid.query(transformA->position, searchRadius);

                for (auto* enemyB : neighbors)
                {
                    if (enemyA == enemyB) continue;
                    // Only separate from other enemies
                    if (enemyB->tag != "Enemy") continue;
                    
                    // Avoid double check
                    if (enemyA->getId() > enemyB->getId()) continue;

                    auto* transformB = enemyB->getComponent<ECS::Components::Transform>();
                    auto* colliderB = enemyB->getComponent<ECS::Components::Collider>();

                    if (!transformB || !colliderB) continue;

                    if (colliderA->intersects(colliderB, transformA->position, transformB->position))
                    {
                        // Calculate separation vector
                        sf::Vector2f direction = transformB->position - transformA->position;
                        float distance = Utils::Math::magnitude(direction);

                        if (distance > 0.f)
                        {
                            direction = Utils::Math::normalize(direction);

                            // Calculate overlap
                            float overlap = 0.f;
                            if (colliderA->shape == ECS::Components::ColliderShape::Circle &&
                                colliderB->shape == ECS::Components::ColliderShape::Circle)
                            {
                                overlap = (colliderA->radius + colliderB->radius) - distance;
                            }
                            else
                            {
                                overlap = 10.f; // Default separation for non-circle
                            }

                            // Push apart
                            sf::Vector2f separation = direction * (overlap * 0.5f);
                            transformA->position -= separation;
                            transformB->position += separation;
                        }
                    }
                }
            }
            Utils::Profiler::stop("Enemy Separation");
        }

        ECS::EntityManager* entityManager;
        float playerDamageCooldown;
        float playerDamageInterval;
        float cullingRange;
        Utils::SpatialGrid grid;
    };
}
