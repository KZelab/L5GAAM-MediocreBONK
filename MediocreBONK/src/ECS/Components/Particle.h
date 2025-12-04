#pragma once
#include "../Component.h"
#include <SFML/Graphics.hpp>

namespace MediocreBONK::ECS::Components
{
    enum class ParticleType
    {
        DamageNumber,
        Explosion,
        Pickup,
        Trail,
        Spark
    };

    class Particle : public Component
    {
    public:
        Particle(ParticleType type, float lifetime, const sf::Vector2f& velocity = sf::Vector2f(0.f, 0.f))
            : type(type)
            , lifetime(lifetime)
            , maxLifetime(lifetime)
            , velocity(velocity)
            , fadeOut(true)
            , gravity(0.f)
            , damping(0.95f)
            , scale(1.f)
            , scaleSpeed(0.f)
            , text("")
            , color(sf::Color::White)
        {}

        void update(sf::Time dt) override
        {
            float deltaTime = dt.asSeconds();

            lifetime -= deltaTime;

            // Apply velocity
            if (owner)
            {
                auto* transform = owner->getComponent<Transform>();
                if (transform)
                {
                    transform->position += velocity * deltaTime;
                    velocity.y += gravity * deltaTime;
                    velocity *= damping;
                }
            }

            // Scale animation
            scale += scaleSpeed * deltaTime;

            // Mark inactive when lifetime expires
            if (lifetime <= 0.f && owner)
            {
                owner->setActive(false);
            }
        }

        float getAlpha() const
        {
            if (fadeOut)
            {
                return (lifetime / maxLifetime) * 255.f;
            }
            return 255.f;
        }

        float getLifetimePercent() const
        {
            return lifetime / maxLifetime;
        }

        ParticleType type;
        float lifetime;
        float maxLifetime;
        sf::Vector2f velocity;
        bool fadeOut;
        float gravity;
        float damping;
        float scale;
        float scaleSpeed;
        std::string text;        // For damage numbers
        sf::Color color;
    };
}
