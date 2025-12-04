#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Collider.h"
#include "../Entities/PowerUp.h"
#include "../Utils/Random.h"
#include "../Utils/Math.h"
#include <SFML/System/Time.hpp>
#include <memory>
#include <vector>

namespace MediocreBONK::Systems
{
    class PowerUpSystem
    {
    public:
        PowerUpSystem(ECS::EntityManager* entityManager, ECS::Entity* player)
            : entityManager(entityManager)
            , player(player)
            , spawnTimer(0.f)
            , spawnInterval(20.f) // Spawn power-up every 20 seconds
        {}

        void update(sf::Time dt)
        {
            spawnTimer += dt.asSeconds();

            // Spawn power-up periodically
            if (spawnTimer >= spawnInterval)
            {
                spawnTimer = 0.f;
                spawnRandomPowerUp();
            }

            // Update all power-ups
            for (auto& powerUp : powerUps)
            {
                if (powerUp && powerUp->getEntity()->isActive())
                {
                    powerUp->update(dt);
                }
            }

            // Check for collection
            checkCollision();

            // Remove inactive power-ups
            powerUps.erase(
                std::remove_if(powerUps.begin(), powerUps.end(),
                    [](const std::unique_ptr<Entities::PowerUp>& p) {
                        return !p || !p->getEntity()->isActive();
                    }),
                powerUps.end()
            );
        }

        void spawnPowerUp(Entities::PowerUpType type, const sf::Vector2f& position)
        {
            auto powerUp = Entities::PowerUpFactory::create(entityManager, type, position);
            if (powerUp)
            {
                powerUps.push_back(std::move(powerUp));
            }
        }

        void setSpawnInterval(float interval)
        {
            spawnInterval = interval;
        }

    private:
        void spawnRandomPowerUp()
        {
            auto* playerTransform = player->getComponent<ECS::Components::Transform>();
            if (!playerTransform)
                return;

            // Spawn near player but not too close
            float distance = Utils::Random::range(200.f, 400.f);
            sf::Vector2f offset = Utils::Random::onCircle(distance);
            sf::Vector2f spawnPos = playerTransform->position + offset;

            // Random power-up type (weighted)
            Entities::PowerUpType type = getRandomPowerUpType();

            spawnPowerUp(type, spawnPos);
        }

        Entities::PowerUpType getRandomPowerUpType()
        {
            float roll = Utils::Random::value();

            // Count entities and XP gems for dynamic spawn rates
            size_t totalEntities = entityManager->getEntityCount();
            size_t xpGemCount = entityManager->getEntitiesByTag("XPGem").size();

            // Calculate dynamic LargeMagnet bonus
            // Base: 2%, increases by 0.1% per 10 entities over 100
            float largeMagnetBonus = 0.f;
            if (totalEntities > 100)
            {
                largeMagnetBonus = (totalEntities - 100) / 100.0f * 0.01f;
            }
            // Cap bonus at 8% (for total of 10%)
            if (largeMagnetBonus > 0.08f)
                largeMagnetBonus = 0.08f;

            // Weighted spawn chances (updated for magnets):
            // 25% Health Pack or small magnet
            // 20% Damage Boost
            // 15% Speed Boost
            // 12% XP Boost
            // 2-10% Large Magnet (dynamic based on entity count)
            // 3% Invulnerability

            float largeMagnetChance = 0.02f + largeMagnetBonus;
            
                
            if (roll < 0.25f)
            {
                if (Utils::Random::chance(0.5f))
                {
					//lets check if health is full, if so, always spawn small magnet instead
                    return Entities::PowerUpType::HealthPack;
                }
                else
                {
                    return Entities::PowerUpType::SmallMagnet;
				}
            }
            else if (roll < 0.45f)
                return Entities::PowerUpType::DamageBoost;
            else if (roll < 0.60f)
                return Entities::PowerUpType::SpeedBoost;
            else if (roll < 0.72f)
                return Entities::PowerUpType::XPBoost;
            else if (roll < 0.80f + largeMagnetChance)
                return Entities::PowerUpType::LargeMagnet;
            else
                return Entities::PowerUpType::InvulnerabilityBoost;
        }

        void checkCollision()
        {
            auto* playerTransform = player->getComponent<ECS::Components::Transform>();
            auto* playerCollider  = player->getComponent<ECS::Components::Collider>();

            if (!playerTransform || !playerCollider)
                return;

            for (auto& powerUp : powerUps)
            {
                if (!powerUp || !powerUp->getEntity()->isActive() || powerUp->isCollected())
                    continue;

                auto* powerUpTransform = powerUp->getEntity()->getComponent<ECS::Components::Transform>();
                auto* powerUpCollider  = powerUp->getEntity()->getComponent<ECS::Components::Collider>();

                if (!powerUpTransform || !powerUpCollider)
                    continue;

                // Check distance for collision
                float distance = Utils::Math::distance(
                    playerTransform->position,
                    powerUpTransform->position
                );

                float combinedRadius = playerCollider->radius + powerUpCollider->radius;

                if (distance < combinedRadius)
                {
                    powerUp->collect(player);
                }
            }
        }

        ECS::EntityManager* entityManager;
        ECS::Entity* player;
        std::vector<std::unique_ptr<Entities::PowerUp>> powerUps;
        float spawnTimer;
        float spawnInterval;
    };
}
