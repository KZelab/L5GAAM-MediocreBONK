#pragma once
#include "../Component.h"
#include <SFML/System/Vector2.hpp>

namespace MediocreBONK::ECS::Components
{
    class Transform : public Component
    {
    public:
        Transform(const sf::Vector2f& position = sf::Vector2f(0.f, 0.f),
                  float rotation = 0.f,
                  const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : position(position)
            , rotation(rotation)
            , scale(scale)
        {}

        sf::Vector2f position;
        float rotation; // In degrees
        sf::Vector2f scale;

        // Helper methods
        void translate(const sf::Vector2f& offset)
        {
            position += offset;
        }

        void rotate(float angle)
        {
            rotation += angle;
        }

        void setScale(float uniformScale)
        {
            scale = sf::Vector2f(uniformScale, uniformScale);
        }
    };
}
