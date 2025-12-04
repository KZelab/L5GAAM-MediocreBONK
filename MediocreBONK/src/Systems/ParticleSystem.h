#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Particle.h"
#include "../Utils/Random.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <iomanip>

namespace MediocreBONK::Systems
{
    class ParticleSystem
    {
    public:
        ParticleSystem(ECS::EntityManager* entityManager)
            : entityManager(entityManager)
        {}

        void update(sf::Time dt)
        {
            // Particles update themselves via component system
            // Just clean up inactive particles here
            auto particles = entityManager->getEntitiesWithComponent<ECS::Components::Particle>();
            for (auto* particle : particles)
            {
                if (!particle->isActive())
                {
                    // Entity will be recycled by EntityManager
                }
            }
        }

        void spawnDamageNumber(const sf::Vector2f& position, float damage)
        {
            auto* entity = entityManager->createEntity();
            if (!entity)
                return;

            // Add components
            auto* transform = entity->addComponent<ECS::Components::Transform>(position);
            auto* particle = entity->addComponent<ECS::Components::Particle>(
                ECS::Components::ParticleType::DamageNumber,
                1.5f,  // Lifetime
                sf::Vector2f(Utils::Random::range(-20.f, 20.f), -100.f)  // Float upward
            );

            // Configure particle
            particle->fadeOut = true;
            particle->gravity = 50.f;  // Slight downward pull after initial float
            particle->text = formatDamage(damage);
            particle->color = sf::Color::White;

            entity->tag = "Particle";
        }

        void spawnExplosion(const sf::Vector2f& position, int particleCount = 20)
        {
            for (int i = 0; i < particleCount; ++i)
            {
                auto* entity = entityManager->createEntity();
                if (!entity)
                    continue;

                // Random velocity in all directions
                float angle = Utils::Random::range(0.f, 360.f) * 3.14159f / 180.f;
                float speed = Utils::Random::range(50.f, 150.f);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

                auto* transform = entity->addComponent<ECS::Components::Transform>(position);
                auto* particle = entity->addComponent<ECS::Components::Particle>(
                    ECS::Components::ParticleType::Explosion,
                    Utils::Random::range(0.5f, 1.f),
                    velocity
                );

                particle->fadeOut = true;
                particle->damping = 0.92f;
                particle->scale = Utils::Random::range(2.f, 5.f);
                particle->scaleSpeed = -2.f;  // Shrink over time

                // Color gradient from orange to red
                float colorMix = Utils::Random::value();
                particle->color = sf::Color(
                    255,
                    static_cast<std::uint8_t>(100 + colorMix * 155),
                    0
                );

                entity->tag = "Particle";
            }
        }

        void spawnPickupEffect(const sf::Vector2f& position)
        {
            int particleCount = 10;
            for (int i = 0; i < particleCount; ++i)
            {
                auto* entity = entityManager->createEntity();
                if (!entity)
                    continue;

                // Spiral outward
                float angle = (i / static_cast<float>(particleCount)) * 360.f * 3.14159f / 180.f;
                float speed = Utils::Random::range(30.f, 80.f);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

                auto* transform = entity->addComponent<ECS::Components::Transform>(position);
                auto* particle = entity->addComponent<ECS::Components::Particle>(
                    ECS::Components::ParticleType::Pickup,
                    0.8f,
                    velocity
                );

                particle->fadeOut = true;
                particle->damping = 0.95f;
                particle->scale = 3.f;
                particle->scaleSpeed = -3.f;
                particle->color = sf::Color::Cyan;

                entity->tag = "Particle";
            }
        }

        void spawnTrail(const sf::Vector2f& position, const sf::Color& color)
        {
            auto* entity = entityManager->createEntity();
            if (!entity)
                return;

            auto* transform = entity->addComponent<ECS::Components::Transform>(position);
            auto* particle = entity->addComponent<ECS::Components::Particle>(
                ECS::Components::ParticleType::Trail,
                0.3f,  // Short lifetime
                sf::Vector2f(0.f, 0.f)
            );

            particle->fadeOut = true;
            particle->scale = 8.f;
            particle->scaleSpeed = -10.f;
            particle->color = color;

            entity->tag = "Particle";
        }

        void spawnSparks(const sf::Vector2f& position, int count = 5)
        {
            for (int i = 0; i < count; ++i)
            {
                auto* entity = entityManager->createEntity();
                if (!entity)
                    continue;

                float angle = Utils::Random::range(0.f, 360.f) * 3.14159f / 180.f;
                float speed = Utils::Random::range(100.f, 200.f);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

                auto* transform = entity->addComponent<ECS::Components::Transform>(position);
                auto* particle = entity->addComponent<ECS::Components::Particle>(
                    ECS::Components::ParticleType::Spark,
                    0.5f,
                    velocity
                );

                particle->fadeOut = true;
                particle->damping = 0.90f;
                particle->gravity = 200.f;
                particle->scale = 2.f;
                particle->color = sf::Color::Yellow;

                entity->tag = "Particle";
            }
        }

        void spawnBuffApplied(const sf::Vector2f& position, const sf::Color& color, int count = 18)
        {
            // Circle burst of particles when buff is applied
            for (int i = 0; i < count; ++i)
            {
                auto* entity = entityManager->createEntity();
                if (!entity)
                    continue;

                // Evenly distributed circle
                float angle = (i / static_cast<float>(count)) * 360.f * 3.14159f / 180.f;
                float speed = Utils::Random::range(80.f, 120.f);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

                auto* transform = entity->addComponent<ECS::Components::Transform>(position);
                auto* particle = entity->addComponent<ECS::Components::Particle>(
                    ECS::Components::ParticleType::Pickup,
                    1.0f,
                    velocity
                );

                particle->fadeOut = true;
                particle->damping = 0.93f;
                particle->scale = 4.f;
                particle->scaleSpeed = -4.f;
                particle->color = color;

                entity->tag = "Particle";
            }
        }

        void spawnBuffExpired(const sf::Vector2f& position, int count = 8)
        {
            // Small gray puff when buff expires
            for (int i = 0; i < count; ++i)
            {
                auto* entity = entityManager->createEntity();
                if (!entity)
                    continue;

                float angle = Utils::Random::range(0.f, 360.f) * 3.14159f / 180.f;
                float speed = Utils::Random::range(20.f, 50.f);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

                auto* transform = entity->addComponent<ECS::Components::Transform>(position);
                auto* particle = entity->addComponent<ECS::Components::Particle>(
                    ECS::Components::ParticleType::Trail,
                    0.6f,
                    velocity
                );

                particle->fadeOut = true;
                particle->damping = 0.95f;
                particle->scale = 3.f;
                particle->scaleSpeed = -2.f;
                particle->color = sf::Color(150, 150, 150);

                entity->tag = "Particle";
            }
        }

        void spawnLevelUp(const sf::Vector2f& position, int count = 30)
        {
            // Gold star burst for level up
            for (int i = 0; i < count; ++i)
            {
                auto* entity = entityManager->createEntity();
                if (!entity)
                    continue;

                float angle = (i / static_cast<float>(count)) * 360.f * 3.14159f / 180.f;
                float speed = Utils::Random::range(100.f, 180.f);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

                auto* transform = entity->addComponent<ECS::Components::Transform>(position);
                auto* particle = entity->addComponent<ECS::Components::Particle>(
                    ECS::Components::ParticleType::Spark,
                    1.2f,
                    velocity
                );

                particle->fadeOut = true;
                particle->damping = 0.88f;
                particle->gravity = 100.f;
                particle->scale = 5.f;
                particle->scaleSpeed = -3.f;
                particle->color = sf::Color(255, 215, 0);  // Gold

                entity->tag = "Particle";
            }
        }

        void render(sf::RenderWindow& window)
        {
            auto particles = entityManager->getEntitiesWithComponent<ECS::Components::Particle>();

            for (auto* entity : particles)
            {
                auto* transform = entity->getComponent<ECS::Components::Transform>();
                auto* particle = entity->getComponent<ECS::Components::Particle>();

                if (!transform || !particle)
                    continue;

                sf::Color color = particle->color;
                color.a = static_cast<std::uint8_t>(particle->getAlpha());

                switch (particle->type)
                {
                case ECS::Components::ParticleType::DamageNumber:
                    // For now, just draw a small circle
                    // In full implementation, would render text
                    {
                        sf::CircleShape shape(3.f * particle->scale);
                        shape.setOrigin({3.f * particle->scale, 3.f * particle->scale});
                        shape.setPosition(transform->position);
                        shape.setFillColor(color);
                        window.draw(shape);
                    }
                    break;

                case ECS::Components::ParticleType::Explosion:
                case ECS::Components::ParticleType::Pickup:
                case ECS::Components::ParticleType::Spark:
                    {
                        sf::CircleShape shape(particle->scale);
                        shape.setOrigin({particle->scale, particle->scale});
                        shape.setPosition(transform->position);
                        shape.setFillColor(color);
                        window.draw(shape);
                    }
                    break;

                case ECS::Components::ParticleType::Trail:
                    {
                        sf::CircleShape shape(particle->scale);
                        shape.setOrigin({particle->scale, particle->scale});
                        shape.setPosition(transform->position);
                        shape.setFillColor(color);
                        window.draw(shape);
                    }
                    break;
                }
            }
        }

    private:
        std::string formatDamage(float damage) const
        {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(0) << damage;
            return ss.str();
        }

        ECS::EntityManager* entityManager;
    };
}
