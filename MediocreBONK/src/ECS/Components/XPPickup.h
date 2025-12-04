#pragma once
#include "../Component.h"
#include "Transform.h"
#include "Buff.h"
#include "../../Utils/Math.h"
#include <SFML/System/Vector2.hpp>

namespace MediocreBONK::ECS::Components
{
    class XPPickup : public Component
    {
    public:
        XPPickup(float value, Entity* player, float magnetRange = 300.f, float pickupRange = 20.f)
            : value(value)
            , player(player)
            , magnetRange(magnetRange)
            , pickupRange(pickupRange)
            , isBeingPulled(false)
            , lifetime(60.f) // Default 60s lifetime
        {}

        void update(sf::Time dt) override
        {
            if (!owner || !player)
                return;

            // Handle lifetime
            lifetime -= dt.asSeconds();
            if (lifetime <= 0.f)
            {
                owner->setActive(false);
                return;
            }

            auto* transform = owner->getComponent<Transform>();
            auto* playerTransform = player->getComponent<Transform>();
            auto* playerBuff = player->getComponent<Buff>();

            if (!transform || !playerTransform)
                return;

            float distance = Utils::Math::distance(transform->position, playerTransform->position);

            // Check if in pickup range
            if (distance <= pickupRange)
            {
                // Will be collected by pickup system
                return;
            }

            // Calculate effective magnet range
            float effectiveMagnetRange = magnetRange;
            if (playerBuff)
            {
                effectiveMagnetRange *= playerBuff->getBuffMultiplier(BuffType::MagnetRange);
            }

            // Check if in magnet range
            if (distance <= effectiveMagnetRange)
            {
                isBeingPulled = true;

                // Lerp toward player
                sf::Vector2f direction = Utils::Math::normalize(playerTransform->position - transform->position);
                float pullSpeed = 300.f; // Speed of magnetic pull
                
                // Increase pull speed if closer
                if (distance < 100.f) pullSpeed *= 2.f;

                transform->position += direction * pullSpeed * dt.asSeconds();
            }
        }

        float getValue() const { return value; }
        float getPickupRange() const { return pickupRange; }
        bool isPulled() const { return isBeingPulled; }

    private:
        float value;
        Entity* player;
        float magnetRange;
        float pickupRange;
        bool isBeingPulled;
        float lifetime;
    };
}
