#pragma once
#include "../Component.h"
#include "Transform.h"
#include "Buff.h"
#include "../../Utils/Math.h"
#include <SFML/System/Vector2.hpp>
#include <cmath>

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
            , readyForPickup(false)
            , lifetime(40.f) // Default 60s lifetime
            , updateTimer(0.f)
            , updateInterval(0.05f) // Update every 50ms instead of every frame
        {}

        void update(sf::Time dt) override
        {
            if (!owner || !player)
                return;

            // Handle lifetime (always update)
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

            // OPTIMIZATION: Throttle expensive distance checks to reduce CPU load
            // Gems don't need to check distance/magnet every single frame
            updateTimer += dt.asSeconds();
            bool shouldUpdateCheck = updateTimer >= updateInterval;
            if (shouldUpdateCheck)
            {
                updateTimer = 0.f; // Reset timer
            }

            // Only perform expensive checks every updateInterval
            if (shouldUpdateCheck)
            {
                // OPTIMIZATION: Quick squared distance check to cull far-away gems
                // This avoids expensive sqrt calculation for gems that are definitely too far
                sf::Vector2f delta = playerTransform->position - transform->position;
                float squaredDistance = delta.x * delta.x + delta.y * delta.y;

                // Calculate effective magnet range for culling
                float cullMagnetRange = magnetRange;
                if (playerBuff)
                {
                    cullMagnetRange *= playerBuff->getBuffMultiplier(BuffType::MagnetRange);
                }
                float cullRangeSquared = cullMagnetRange * cullMagnetRange;

                // Skip update if way beyond magnet range (not moving, won't be collected)
                if (squaredDistance > cullRangeSquared)
                {
                    readyForPickup = false;
                    isBeingPulled = false;
                    return;
                }

                // Only calculate actual distance if within range
                float distance = std::sqrt(squaredDistance);

                // Check if in pickup range
                if (distance <= pickupRange)
                {
                    // Will be collected by pickup system
                    readyForPickup = true;
                    isBeingPulled = false; // Stop pulling if already in pickup range
                    return;
                }
                else
                {
                    readyForPickup = false;
                }

                // Check if in magnet range
                if (distance <= cullMagnetRange)
                {
                    isBeingPulled = true;
                }
                else
                {
                    isBeingPulled = false;
                }
            }

            // Apply movement if being pulled (every frame for smooth movement)
            if (isBeingPulled)
            {
                sf::Vector2f direction = Utils::Math::normalize(playerTransform->position - transform->position);
                float pullSpeed = 300.f; // Speed of magnetic pull

                // Increase pull speed if closer (approximate check without distance calc)
                sf::Vector2f delta = playerTransform->position - transform->position;
                float squaredDist = delta.x * delta.x + delta.y * delta.y;
                if (squaredDist < 100.f * 100.f) pullSpeed *= 2.f;

                transform->position += direction * pullSpeed * dt.asSeconds();
            }
        }

        float getValue() const { return value; }
        float getPickupRange() const { return pickupRange; }
        bool isPulled() const { return isBeingPulled; }
        bool isReadyForPickup() const { return readyForPickup; }

        // OPTIMIZATION: Allow merging XP gems
        void addValue(float additionalValue) { value += additionalValue; }

    private:
        float value;
        Entity* player;
        float magnetRange;
        float pickupRange;
        bool isBeingPulled;
        bool readyForPickup;
        float lifetime;
        float updateTimer;
        float updateInterval;
    };
}
