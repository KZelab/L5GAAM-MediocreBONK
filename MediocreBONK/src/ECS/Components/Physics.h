#pragma once
#include "../Component.h"
#include "Transform.h"
#include <SFML/System/Vector2.hpp>

namespace MediocreBONK::ECS::Components
{
    class Physics : public Component
    {
    public:
        Physics(float mass = 1.f, float drag = 0.98f)
            : velocity(0.f, 0.f)
            , acceleration(0.f, 0.f)
            , mass(mass)
            , drag(drag)
            , maxSpeed(-1.f) // -1 means no limit
        {}

        void update(sf::Time dt) override
        {
            float deltaSeconds = dt.asSeconds();

            // Apply acceleration to velocity
            velocity += acceleration * deltaSeconds;

            // Apply drag
            velocity *= drag;

            // Clamp to max speed if set
            if (maxSpeed > 0.f)
            {
                float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
                if (speed > maxSpeed)
                {
                    velocity = (velocity / speed) * maxSpeed;
                }
            }

            // Update transform position
            if (owner)
            {
                auto* transform = owner->getComponent<Transform>();
                if (transform)
                {
                    transform->position += velocity * deltaSeconds;
                }
            }

            // Reset acceleration for next frame
            acceleration = sf::Vector2f(0.f, 0.f);
        }

        void applyForce(const sf::Vector2f& force)
        {
            // F = ma, so a = F/m
            acceleration += force / mass;
        }

        void applyImpulse(const sf::Vector2f& impulse)
        {
            // Instant velocity change
            velocity += impulse / mass;
        }

        void setVelocity(const sf::Vector2f& newVelocity)
        {
            velocity = newVelocity;
        }

        sf::Vector2f getVelocity() const
        {
            return velocity;
        }

        sf::Vector2f velocity;
        sf::Vector2f acceleration;
        float mass;
        float drag;

        float maxSpeed;
    };
}
