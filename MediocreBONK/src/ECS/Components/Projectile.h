#pragma once
#include "../Component.h"
#include "Transform.h"
#include <vector>
#include <cstdint>

namespace MediocreBONK::ECS::Components
{
    class Projectile : public Component
    {
    public:
        Projectile(float damage, int piercing, float lifetime, const std::string& ownerTag)
            : damage(damage)
            , piercing(piercing)
            , maxPiercing(piercing)
            , lifetime(lifetime)
            , maxLifetime(lifetime)
            , ownerTag(ownerTag)
        {}

        void update(sf::Time dt) override
        {
            lifetime -= dt.asSeconds();

            // Deactivate if lifetime expired
            if (lifetime <= 0.f && owner)
            {
                owner->setActive(false);
            }
        }

        bool canHit(uint64_t entityId)
        {
            // Check if we've already hit this entity
            for (uint64_t hitId : hitEntities)
            {
                if (hitId == entityId)
                    return false;
            }
            return true;
        }

        void recordHit(uint64_t entityId)
        {
            hitEntities.push_back(entityId);
            piercing--;

            // Deactivate if no piercing left
            if (piercing <= 0 && owner)
            {
                owner->setActive(false);
            }
        }

        float getDamage() const { return damage; }
        int getPiercing() const { return piercing; }
        float getLifetimePercent() const { return lifetime / maxLifetime; }
        const std::string& getOwnerTag() const { return ownerTag; }

    private:
        float damage;
        int piercing;
        int maxPiercing;
        float lifetime;
        float maxLifetime;
        std::string ownerTag; // "Player" or "Enemy" to prevent friendly fire
        std::vector<uint64_t> hitEntities;
    };
}
