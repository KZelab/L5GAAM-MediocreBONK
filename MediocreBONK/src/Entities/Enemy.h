#pragma once
#include "../ECS/Entity.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Physics.h"
#include "../ECS/Components/Health.h"
#include "../ECS/Components/AI.h"
#include "../ECS/Components/Collider.h"
#include <SFML/Graphics.hpp>

// Forward declaration to avoid circular dependency
namespace MediocreBONK::Managers
{
    class DifficultyManager;
}

namespace MediocreBONK::Entities
{
    enum class EnemyType
    {
        Light,
        Medium,
        Heavy
    };

    struct EnemyData
    {
        std::string name;
        float maxHealth;
        float speed;
        float damage;
        float experienceValue;
        float radius; // For collider
        sf::Color color; // Visual distinction
        EnemyType type;
    };

    class Enemy
    {
    public:
        Enemy(ECS::Entity* entity, const sf::Vector2f& position, const EnemyData& data, ECS::Entity* player)
            : entity(entity)
            , data(data)
            , onDeathXPCallback(nullptr)
        {
            // Add components
            transform = entity->addComponent<ECS::Components::Transform>(position);
            physics = entity->addComponent<ECS::Components::Physics>(1.f, 0.9f);
            health = entity->addComponent<ECS::Components::Health>(data.maxHealth);
            ai = entity->addComponent<ECS::Components::AI>(ECS::Components::AIBehavior::ChasePlayer, data.speed);
            collider = entity->addComponent<ECS::Components::Collider>(ECS::Components::ColliderShape::Circle, data.radius);

            // Set AI target
            ai->setTarget(player);

            // Set up health callbacks
            health->onDeathCallback = [this]() { onDeath(); };
            health->onDamageCallback = [this](float damage) { onDamage(damage); };

            entity->tag = "Enemy";
        }

        virtual ~Enemy() = default;

        virtual void update(sf::Time dt)
        {
            // Update handled by components automatically
        }

        virtual void onSpawn()
        {
            // Override in derived classes for spawn effects
        }

        virtual void onDeath()
        {
            // Spawn XP gem through callback
            if (onDeathXPCallback && transform)
            {
                onDeathXPCallback(transform->position, data.experienceValue);
            }

            // Play death animation/particles 

            // Deactivate entity
            entity->setActive(false);
        }

        void setOnDeathXPCallback(std::function<void(sf::Vector2f, float)> callback)
        {
            onDeathXPCallback = callback;
        }

        virtual void onDamage(float damage)
        {
            // Flash effect or damage animation (will implement later)
        }

        ECS::Entity* getEntity() { return entity; }
        const EnemyData& getData() const { return data; }

    protected:
        ECS::Entity* entity;
        ECS::Components::Transform* transform;
        ECS::Components::Physics* physics;
        ECS::Components::Health* health;
        ECS::Components::AI* ai;
        ECS::Components::Collider* collider;
        EnemyData data;

        std::function<void(sf::Vector2f, float)> onDeathXPCallback;
    };

    // Factory class for creating enemies
    class EnemyFactory
    {
    public:
        static EnemyData getLightEnemyData()
        {
            EnemyData data;
            data.name = "Light";
            data.maxHealth = 10.f;
            data.speed = 150.f;
            data.damage = 5.f;
            data.experienceValue = 1.f; 
            data.radius = 15.f;
            data.color = sf::Color(255, 100, 100); // Light red
            data.type = EnemyType::Light;
            return data;
        }

        static EnemyData getMediumEnemyData()
        {
            EnemyData data;
            data.name = "Medium";
            data.maxHealth = 30.f;
            data.speed = 100.f;
            data.damage = 10.f;
            data.experienceValue = 5.f; // Reduced from 15 for balanced progression
            data.radius = 30.f;
            data.color = sf::Color(255, 150, 100); // Orange
            data.type = EnemyType::Medium;
            return data;
        }

        static EnemyData getHeavyEnemyData()
        {
            EnemyData data;
            data.name = "Heavy";
            data.maxHealth = 100.f;
            data.speed = 50.f;
            data.damage = 20.f;
            data.experienceValue = 20.f; // Reduced from 50 for balanced progression
            data.radius = 45.f;
            data.color = sf::Color(200, 50, 50); // Dark red
            data.type = EnemyType::Heavy;
            return data;
        }

        static std::unique_ptr<Enemy> create(ECS::EntityManager* entityManager,
                                             EnemyType type,
                                             const sf::Vector2f& position,
                                             ECS::Entity* player);
    };
}

// Include DifficultyManager here to avoid circular dependency
#include "../Managers/DifficultyManager.h"

namespace MediocreBONK::Entities
{
    inline std::unique_ptr<Enemy> EnemyFactory::create(ECS::EntityManager* entityManager,
                                                         EnemyType type,
                                                         const sf::Vector2f& position,
                                                         ECS::Entity* player)
    {
        auto* entity = entityManager->createEntity();
        if (!entity)
            return nullptr;

        EnemyData data;
        switch (type)
        {
        case EnemyType::Light:
            data = getLightEnemyData();
            break;
        case EnemyType::Medium:
            data = getMediumEnemyData();
            break;
        case EnemyType::Heavy:
            data = getHeavyEnemyData();
            break;
        }

        // Apply difficulty scaling
        auto& difficulty = Managers::DifficultyManager::getInstance();
        data.maxHealth *= difficulty.getHealthMultiplier();
        data.speed *= difficulty.getSpeedMultiplier();
        data.damage *= difficulty.getDamageMultiplier();
        data.experienceValue *= difficulty.getXPMultiplier();

        // Apply type-specific multipliers (stacks with difficulty)
        // Medium and Heavy enemies get additional bonuses to speed and resilience
        switch (type)
        {
        case EnemyType::Light:
            // Light enemies have no bonus
            break;
        case EnemyType::Medium:
            // Medium enemies: +25% health, +15% speed, +20% damage
            data.maxHealth *= 1.25f;
            data.speed *= 1.15f;
            data.damage *= 1.20f;
            data.experienceValue *= 1.2f; // More XP to compensate
            break;
        case EnemyType::Heavy:
            // Heavy enemies: +50% health, +20% speed, +40% damage
            data.maxHealth *= 1.5f;
            data.speed *= 1.20f;
            data.damage *= 1.40f;
            data.experienceValue *= 1.4f; // More XP to compensate
            break;
        }

        return std::make_unique<Enemy>(entity, position, data, player);
    }
}
