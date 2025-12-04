#pragma once
#include "../ECS/Entity.h"
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/Buff.h"
#include "../ECS/Components/XPPickup.h"
#include "../ECS/Components/Experience.h"
#include "../Managers/EventManager.h"
#include "../Utils/Math.h"
#include <SFML/Graphics.hpp>
#include <functional>

namespace MediocreBONK::Entities
{
    enum class PowerUpType
    {
        HealthPack,
        DamageBoost,
        SpeedBoost,
        InvulnerabilityBoost,
        XPBoost,
        SmallMagnet,
        LargeMagnet
    };

    struct PowerUpData
    {
        PowerUpType type;
        std::string name;
        float value;
        float duration;  // -1 for permanent
        sf::Color color;
        float radius;
    };

    class PowerUp
    {
    public:
        PowerUp(ECS::Entity* entity, const sf::Vector2f& position, const PowerUpData& data, ECS::EntityManager* entityManager)
            : entity(entity)
            , data(data)
            , entityManager(entityManager)
            , lifetime(30.f) // PowerUps despawn after 30 seconds
            , collected(false)
        {
            // Add components
            transform = entity->addComponent<ECS::Components::Transform>(position);
            collider = entity->addComponent<ECS::Components::Collider>(
                ECS::Components::ColliderShape::Circle, data.radius);

            entity->tag = "PowerUp";
        }

        void update(sf::Time dt)
        {
            lifetime -= dt.asSeconds();
            if (lifetime <= 0.f)
            {
                entity->setActive(false);
            }

            // Floating animation (simple sine wave)
            if (transform)
            {
                float floatOffset = std::sin(lifetime * 3.f) * 5.f;
                transform->position.y += floatOffset * dt.asSeconds();
            }
        }

        void collect(ECS::Entity* player)
        {
            if (collected)
                return;

            collected = true;

            // Apply power-up effect
            applyEffect(player);

            // Emit event
            auto eventData = std::make_unique<Managers::BuffAppliedData>();
            eventData->buffName = data.name;
            eventData->duration = data.duration;
            Managers::EventManager::getInstance().queueEvent(
                Managers::GameEventType::PowerUpCollected, std::move(eventData));

            // Deactivate entity
            entity->setActive(false);
        }

        PowerUpType getType() const { return data.type; }
        const PowerUpData& getData() const { return data; }
        ECS::Entity* getEntity() { return entity; }
        bool isCollected() const { return collected; }

    private:
        void applyEffect(ECS::Entity* player)
        {
            switch (data.type)
            {
            case PowerUpType::HealthPack:
            {
                auto* health = player->getComponent<ECS::Components::Health>();
                if (health)
                    health->heal(data.value);
                break;
            }
            case PowerUpType::DamageBoost:
            {
                auto* buff = player->getComponent<ECS::Components::Buff>();
                if (!buff)
                    buff = player->addComponent<ECS::Components::Buff>();

                ECS::Components::BuffEffect damageBoost(
                    "Damage Boost", ECS::Components::BuffType::DamageBoost,
                    data.value, data.duration);
                buff->addBuff(damageBoost);
                break;
            }
            case PowerUpType::SpeedBoost:
            {
                auto* buff = player->getComponent<ECS::Components::Buff>();
                if (!buff)
                    buff = player->addComponent<ECS::Components::Buff>();

                ECS::Components::BuffEffect speedBoost(
                    "Speed Boost", ECS::Components::BuffType::SpeedBoost,
                    data.value, data.duration);
                buff->addBuff(speedBoost);
                break;
            }
            case PowerUpType::InvulnerabilityBoost:
            {
                auto* buff = player->getComponent<ECS::Components::Buff>();
                if (!buff)
                    buff = player->addComponent<ECS::Components::Buff>();

                ECS::Components::BuffEffect invulnBoost(
                    "Invulnerability", ECS::Components::BuffType::InvulnerabilityBoost,
                    1.0f, data.duration);
                buff->addBuff(invulnBoost);
                break;
            }
            case PowerUpType::XPBoost:
            {
                auto* buff = player->getComponent<ECS::Components::Buff>();
                if (!buff)
                    buff = player->addComponent<ECS::Components::Buff>();

                ECS::Components::BuffEffect xpBoost(
                    "XP Boost", ECS::Components::BuffType::XPMultiplier,
                    data.value, data.duration);
                buff->addBuff(xpBoost);
                break;
            }
            case PowerUpType::SmallMagnet:
            {
                // Instantly collect all XP gems within a small radius
                auto* playerTransform = player->getComponent<ECS::Components::Transform>();
                auto* playerExperience = player->getComponent<ECS::Components::Experience>();

                if (playerTransform && playerExperience && entityManager)
                {
                    auto xpGems = entityManager->getEntitiesByTag("XPGem");
                    float collectRadius = data.value; // Use value as radius

                    for (auto* gem : xpGems)
                    {
                        auto* gemTransform = gem->getComponent<ECS::Components::Transform>();
                        auto* xpPickup = gem->getComponent<ECS::Components::XPPickup>();

                        if (gemTransform && xpPickup)
                        {
                            float distance = Utils::Math::distance(
                                playerTransform->position,
                                gemTransform->position
                            );

                            if (distance <= collectRadius)
                            {
                                // Collect the gem
                                float xpValue = xpPickup->getValue();
                                auto* buff = player->getComponent<ECS::Components::Buff>();
                                if (buff)
                                {
                                    xpValue *= buff->getBuffMultiplier(ECS::Components::BuffType::XPMultiplier);
                                }
                                playerExperience->addXP(xpValue);
                                gem->setActive(false);
                            }
                        }
                    }
                }
                break;
            }
            case PowerUpType::LargeMagnet:
            {
                // Instantly collect ALL XP gems on screen
                auto* playerExperience = player->getComponent<ECS::Components::Experience>();

                if (playerExperience && entityManager)
                {
                    auto xpGems = entityManager->getEntitiesByTag("XPGem");

                    for (auto* gem : xpGems)
                    {
                        auto* xpPickup = gem->getComponent<ECS::Components::XPPickup>();

                        if (xpPickup)
                        {
                            // Collect the gem
                            float xpValue = xpPickup->getValue();
                            auto* buff = player->getComponent<ECS::Components::Buff>();
                            if (buff)
                            {
                                xpValue *= buff->getBuffMultiplier(ECS::Components::BuffType::XPMultiplier);
                            }
                            playerExperience->addXP(xpValue);
                            gem->setActive(false);
                        }
                    }
                }
                break;
            }
            }
        }

        ECS::Entity* entity;
        ECS::EntityManager* entityManager;
        ECS::Components::Transform* transform;
        ECS::Components::Collider* collider;
        PowerUpData data;
        float lifetime;
        bool collected;
    };

    // Factory for creating power-ups
    class PowerUpFactory
    {
    public:
        static PowerUpData getHealthPackData()
        {
            return PowerUpData{
                PowerUpType::HealthPack,
                "Health Pack",
                25.f,      // Heal amount
                -1.f,      // Instant effect
                sf::Color::Green,
                12.f
            };
        }

        static PowerUpData getDamageBoostData()
        {
            return PowerUpData{
                PowerUpType::DamageBoost,
                "Damage Boost",
                0.5f,      // +50% damage
                10.f,      // 10 seconds
                sf::Color::Red,
                12.f
            };
        }

        static PowerUpData getSpeedBoostData()
        {
            return PowerUpData{
                PowerUpType::SpeedBoost,
                "Speed Boost",
                0.5f,      // +50% speed
                10.f,      // 10 seconds
                sf::Color::Cyan,
                12.f
            };
        }

        static PowerUpData getInvulnerabilityData()
        {
            return PowerUpData{
                PowerUpType::InvulnerabilityBoost,
                "Invulnerability",
                1.0f,      // Full invulnerability
                5.f,       // 5 seconds
                sf::Color::Yellow,
                12.f
            };
        }

        static PowerUpData getXPBoostData()
        {
            return PowerUpData{
                PowerUpType::XPBoost,
                "XP Boost",
                1.0f,      // +100% XP
                15.f,      // 15 seconds
                sf::Color::Magenta,
                12.f
            };
        }

        static PowerUpData getSmallMagnetData()
        {
            return PowerUpData{
                PowerUpType::SmallMagnet,
                "Small Magnet",
                200.f,     // Collect radius
                -1.f,      // Instant effect
                sf::Color(150, 150, 255), // Light blue
                12.f
            };
        }

        static PowerUpData getLargeMagnetData()
        {
            return PowerUpData{
                PowerUpType::LargeMagnet,
                "Large Magnet",
                0.f,       // Not used (collects all)
                -1.f,      // Instant effect
                sf::Color(100, 100, 255), // Darker blue
                15.f
            };
        }

        static std::unique_ptr<PowerUp> create(ECS::EntityManager* entityManager,
                                               PowerUpType type,
                                               const sf::Vector2f& position)
        {
            auto* entity = entityManager->createEntity();
            if (!entity)
                return nullptr;

            PowerUpData data;
            switch (type)
            {
            case PowerUpType::HealthPack:
                data = getHealthPackData();
                break;
            case PowerUpType::DamageBoost:
                data = getDamageBoostData();
                break;
            case PowerUpType::SpeedBoost:
                data = getSpeedBoostData();
                break;
            case PowerUpType::InvulnerabilityBoost:
                data = getInvulnerabilityData();
                break;
            case PowerUpType::XPBoost:
                data = getXPBoostData();
                break;
            case PowerUpType::SmallMagnet:
                data = getSmallMagnetData();
                break;
            case PowerUpType::LargeMagnet:
                data = getLargeMagnetData();
                break;
            }

            return std::make_unique<PowerUp>(entity, position, data, entityManager);
        }
    };
}
