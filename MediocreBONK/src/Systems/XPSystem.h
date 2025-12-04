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
            auto* gem = entityManager->createEntity();
            if (!gem)
                return;

            gem->tag = "XPGem";

            // Add components
            auto* transform = gem->addComponent<ECS::Components::Transform>(position);
            auto* xpPickup = gem->addComponent<ECS::Components::XPPickup>(xpValue, player, 100.f, 30.f);
            auto* collider = gem->addComponent<ECS::Components::Collider>(ECS::Components::ColliderShape::Circle, 10.f);

            // Add some randomness to spawn position (scatter effect)
            sf::Vector2f scatter = Utils::Random::insideCircle(20.f);
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
                auto* gemTransform = gem->getComponent<ECS::Components::Transform>();
                auto* xpPickup = gem->getComponent<ECS::Components::XPPickup>();

                if (!gemTransform || !xpPickup)
                    continue;

                float distance = Utils::Math::distance(playerTransform->position, gemTransform->position);

                // Check if player is close enough to collect
                if (distance <= xpPickup->getPickupRange())
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
