#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Weapon.h"
#include "../ECS/Components/Physics.h"
#include "../ECS/Components/Projectile.h"
#include "../ECS/Components/Collider.h"
#include "../Utils/Math.h"
#include "../Utils/Logger.h"
#include <SFML/Graphics.hpp>

namespace MediocreBONK::Systems
{
    class WeaponSystem
    {
    public:
        WeaponSystem(ECS::EntityManager* entityManager)
            : entityManager(entityManager)
        {}

        void update(sf::Time dt)
        {
            // Get all entities with weapons
            auto weaponEntities = entityManager->getEntitiesWithComponents<
                ECS::Components::Transform,
                ECS::Components::Weapon
            >();

            static bool loggedWeaponCount = false;
            if (!loggedWeaponCount)
            {
                Utils::Logger::info("Found " + std::to_string(weaponEntities.size()) + " entities with weapons");
                loggedWeaponCount = true;
            }

            for (auto* entity : weaponEntities)
            {
                auto* transform = entity->getComponent<ECS::Components::Transform>();
                auto* weapon    = entity->getComponent<ECS::Components::Weapon>();

                // Handle burst firing
                if (weapon->burstShotsRemaining > 0)
                {
                    weapon->burstTimer -= dt.asSeconds();
                    if (weapon->burstTimer <= 0.f)
                    {
                        // Fire next shot in burst
                        ECS::Entity* target = findNearestTarget(entity);
                        if (target)
                        {
                            int index = weapon->data.projectileCount - weapon->burstShotsRemaining;
                            fireSingleShot(entity, transform, weapon, target, index);
                            weapon->burstShotsRemaining--;
                            weapon->burstTimer = weapon->burstDelay;
                        }
                        else
                        {
                            // No target, cancel burst
                            weapon->burstShotsRemaining = 0;
                        }
                    }
                }

                if (!weapon->autoFire)
                    continue;

                // Try to fire weapon (starts a new burst)
                if (weapon->tryFire())
                {
                    // Find nearest enemy to target
                    ECS::Entity* target = findNearestTarget(entity);

                    if (target)
                    {
                        startFiring(entity, transform, weapon, target);
                    }
                }
            }
        }

    private:
        ECS::Entity* findNearestTarget(ECS::Entity* source)
        {
            auto* sourceTransform = source->getComponent<ECS::Components::Transform>();
            if (!sourceTransform)
                return nullptr;

            std::string targetTag = (source->tag == "Player") ? "Enemy" : "Player";
            auto targets = entityManager->getEntitiesByTag(targetTag);

            ECS::Entity* nearest = nullptr;
            float nearestDistance = std::numeric_limits<float>::max();

            for (auto* target : targets)
            {
                auto* targetTransform = target->getComponent<ECS::Components::Transform>();
                if (!targetTransform)
                    continue;

                float distance = Utils::Math::distance(sourceTransform->position, targetTransform->position);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearest = target;
                }
            }

            return nearest;
        }

        void startFiring(ECS::Entity* source, ECS::Components::Transform* transform,
                       ECS::Components::Weapon* weapon, ECS::Entity* target)
        {
            // Initialize burst
            weapon->burstShotsRemaining = weapon->data.projectileCount;
            
            // Fire first shot immediately
            int index = 0;
            fireSingleShot(source, transform, weapon, target, index);
            weapon->burstShotsRemaining--;
            weapon->burstTimer = weapon->burstDelay;
        }

        void fireSingleShot(ECS::Entity* source, ECS::Components::Transform* transform,
                           ECS::Components::Weapon* weapon, ECS::Entity* target, int index)
        {
            auto* targetTransform = target->getComponent<ECS::Components::Transform>();
            if (!targetTransform)
                return;

            // Calculate direction to target
            sf::Vector2f direction = Utils::Math::normalize(targetTransform->position - transform->position);

            // If we have a delay, we want a stream (ignore spread).
            // If delay is 0 (shotgun), we want spread.
            bool ignoreSpread = (weapon->burstDelay > 0.f);

            createProjectile(source, transform->position, direction, weapon, index, ignoreSpread);
        }

        void createProjectile(ECS::Entity* source, const sf::Vector2f& position,
                             const sf::Vector2f& baseDirection,
                             ECS::Components::Weapon* weapon, int index, bool ignoreSpread = false)
        {
            // Create projectile entity
            auto* projectile = entityManager->createEntity();
            if (!projectile)
                return;

            projectile->tag = source->tag + "Projectile";

            // Calculate spread angle
            float spreadAngle = 0.f;
            if (!ignoreSpread && weapon->data.projectileCount > 1)
            {
                float totalSpread = weapon->data.spread;
                float angleStep = totalSpread / (weapon->data.projectileCount - 1);
                spreadAngle = -totalSpread / 2.f + angleStep * index;
            }

            // Apply spread to direction
            float radians = Utils::Math::toRadians(spreadAngle);
            float cos = std::cos(radians);
            float sin = std::sin(radians);
            sf::Vector2f direction(
                baseDirection.x * cos - baseDirection.y * sin,
                baseDirection.x * sin + baseDirection.y * cos
            );

            // Add components
            auto* transform = projectile->addComponent<ECS::Components::Transform>(position);
            auto* physics = projectile->addComponent<ECS::Components::Physics>();
            physics->velocity = direction * weapon->data.projectileSpeed;
            physics->drag = 1.0f; // No drag for projectiles

            // Calculate lifetime from range
            float lifetime = weapon->data.range / weapon->data.projectileSpeed;
            auto* projComp = projectile->addComponent<ECS::Components::Projectile>(
                weapon->data.damage,
                weapon->data.piercing,
                lifetime,
                source->tag
            );

            // Add collider for collision detection (small circle)
            auto* collider = projectile->addComponent<ECS::Components::Collider>(
                ECS::Components::ColliderShape::Circle,
                5.f  // Small 5 pixel radius for projectiles
            );

            static bool loggedProjectileCreation = false;
            if (!loggedProjectileCreation)
            {
                Utils::Logger::info("Created projectile with damage: " + std::to_string(weapon->data.damage) +
                                   ", speed: " + std::to_string(weapon->data.projectileSpeed) +
                                   ", owner: " + source->tag);
                loggedProjectileCreation = true;
            }

            // Add visual (simple circle for now, will use sprite later)
            // auto* sprite = projectile->addComponent<ECS::Components::Sprite>(weapon->data.projectileSprite, 2);
        }

        ECS::EntityManager* entityManager;
    };
}
