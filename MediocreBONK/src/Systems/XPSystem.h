#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/XPPickup.h"
#include "../ECS/Components/Experience.h"
#include "../ECS/Components/Experience.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/Buff.h"
#include "../Utils/Math.h"
#include "../Utils/Random.h"
#include <SFML/Graphics.hpp>

namespace MediocreBONK::Systems
{
    class XPSystem
    {
    public:
        XPSystem(ECS::EntityManager* entityManager, ECS::Entity* player)
            : entityManager(entityManager)
            , player(player)
        {}

        void update(sf::Time dt)
        {
            // Handle XP pickup collection
            collectXPPickups();
        }

        void spawnXPGem(const sf::Vector2f& position, float xpValue)
        {
            // OPTIMIZATION: Check for nearby gems to merge with (reduces entity count)
            const float MERGE_RADIUS = 25.f; // Merge gems within this radius
            const float MERGE_RADIUS_SQUARED = MERGE_RADIUS * MERGE_RADIUS;

            auto xpGems = entityManager->getEntitiesByTag("XPGem");
            for (auto* existingGem : xpGems)
            {
                auto* existingTransform = existingGem->getComponent<ECS::Components::Transform>();
                auto* existingPickup = existingGem->getComponent<ECS::Components::XPPickup>();

                if (existingTransform && existingPickup)
                {
                    sf::Vector2f delta = existingTransform->position - position;
                    float distSquared = delta.x * delta.x + delta.y * delta.y;

                    if (distSquared <= MERGE_RADIUS_SQUARED)
                    {
                        // Merge into existing gem by increasing its value
                        existingPickup->addValue(xpValue);
                        return; // Don't spawn new gem, we merged instead
                    }
                }
            }

            // OPTIMIZATION: Cap max XP gems to prevent performance issues
            const size_t MAX_XP_GEMS = 150;
            if (xpGems.size() >= MAX_XP_GEMS)
            {
                // Find oldest gem (furthest from player) and replace it
                auto* playerTransform = player->getComponent<ECS::Components::Transform>();
                if (playerTransform)
                {
                    ECS::Entity* furthestGem = nullptr;
                    float maxDistSquared = 0.f;

                    for (auto* gem : xpGems)
                    {
                        auto* gemTransform = gem->getComponent<ECS::Components::Transform>();
                        if (gemTransform)
                        {
                            sf::Vector2f delta = gemTransform->position - playerTransform->position;
                            float distSquared = delta.x * delta.x + delta.y * delta.y;

                            if (distSquared > maxDistSquared)
                            {
                                maxDistSquared = distSquared;
                                furthestGem = gem;
                            }
                        }
                    }

                    if (furthestGem)
                    {
                        furthestGem->setActive(false);
                    }
                }
            }

            auto* gem = entityManager->createEntity();
            if (!gem)
                return;

            gem->tag = "XPGem";

            // Add components
            auto* transform = gem->addComponent<ECS::Components::Transform>(position);
            auto* xpPickup = gem->addComponent<ECS::Components::XPPickup>(xpValue, player, 100.f, 30.f);
            auto* collider = gem->addComponent<ECS::Components::Collider>(ECS::Components::ColliderShape::Circle, 10.f);

            // Add some randomness to spawn position (scatter effect)
            sf::Vector2f scatter = Utils::Random::insideCircle(10.f);
            transform->position += scatter;
        }

    private:
        void collectXPPickups()
        {
            auto* playerTransform = player->getComponent<ECS::Components::Transform>();
            auto* playerExperience = player->getComponent<ECS::Components::Experience>();

            if (!playerTransform || !playerExperience)
                return;

            auto xpGems = entityManager->getEntitiesByTag("XPGem");

            for (auto* gem : xpGems)
            {
                auto* xpPickup = gem->getComponent<ECS::Components::XPPickup>();

                if (!xpPickup)
                    continue;

                // Check if gem is ready for pickup (distance already calculated in XPPickup::update)
                if (xpPickup->isReadyForPickup())
                {
                    // Add XP to player
                    float xpValue = xpPickup->getValue();
                    auto* buff = player->getComponent<ECS::Components::Buff>();
                    if (buff)
                    {
                        xpValue *= buff->getBuffMultiplier(ECS::Components::BuffType::XPMultiplier);
                    }
                    playerExperience->addXP(xpValue);

                    // Deactivate gem
                    gem->setActive(false);

                    // TODO: Play pickup sound/particle effect
                }
            }
        }

        ECS::EntityManager* entityManager;
        ECS::Entity* player;
    };
}
