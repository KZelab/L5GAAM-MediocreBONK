#pragma once
#include "../Component.h"
#include <SFML/System/Time.hpp>
#include <string>

namespace MediocreBONK::ECS::Components
{
    struct WeaponData
    {
        std::string name;
        float damage;
        float fireRate;          // shots per second
        float projectileSpeed;
        int piercing;            // enemies pierced before destroyed
        int projectileCount;     // projectiles per shot
        float spread;            // angle spread for multiple projectiles (degrees)
        float range;             // max distance or lifetime
        std::string projectileSprite;
    };

    class Weapon : public Component
    {
    public:
        Weapon(const WeaponData& data)
            : data(data)
            , fireTimer(0.f)
            , canFire(true)
            , autoFire(true)
            , burstShotsRemaining(0)
            , burstTimer(0.f)
            , burstDelay(0.05f) // 50ms delay between burst shots
        {}

        void update(sf::Time dt) override
        {
            // Update fire timer
            if (!canFire)
            {
                fireTimer += dt.asSeconds();
                float fireDelay = 1.f / data.fireRate;

                if (fireTimer >= fireDelay)
                {
                    canFire = true;
                    fireTimer = 0.f;
                }
            }
        }

        bool tryFire()
        {
            if (canFire)
            {
                canFire = false;
                fireTimer = 0.f;
                return true;
            }
            return false;
        }

        void resetFireCooldown()
        {
            canFire = true;
            fireTimer = 0.f;
        }

        // Upgrade methods (for later implementation)
        void upgradeDamage(float amount)
        {
            data.damage += amount;
        }

        void upgradeFireRate(float amount)
        {
            data.fireRate += amount;
        }

        void upgradeProjectileCount(int amount)
        {
            data.projectileCount += amount;
        }

        void upgradePiercing(int amount)
        {
            data.piercing += amount;
        }

        WeaponData data;
        bool autoFire;
        int burstShotsRemaining;
        float burstTimer;
        float burstDelay;

    private:
        float fireTimer;
        bool canFire;
    };
}
