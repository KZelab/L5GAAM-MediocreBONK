#pragma once
#include "../Component.h"
#include "Transform.h"
#include "../../Utils/Math.h"
#include <SFML/Graphics.hpp>
#include <functional>

namespace MediocreBONK::ECS::Components
{
    enum class ColliderShape
    {
        Circle,
        Rectangle
    };

    class Collider : public Component
    {
    public:
        Collider(ColliderShape shape, float radius)
            : shape(shape)
            , radius(radius)
            , size(radius * 2.f, radius * 2.f)
            , layer(1)
            , mask(0xFFFFFFFF) // Collide with all layers by default
            , isTrigger(false)
        {}

        Collider(ColliderShape shape, const sf::Vector2f& size)
            : shape(shape)
            , radius(0.f)
            , size(size)
            , layer(1)
            , mask(0xFFFFFFFF)
            , isTrigger(false)
        {}

        bool intersects(const Collider* other, const sf::Vector2f& thisPos, const sf::Vector2f& otherPos) const
        {
            // Check layer mask
            if ((layer & other->mask) == 0 && (other->layer & mask) == 0)
                return false;

            if (shape == ColliderShape::Circle && other->shape == ColliderShape::Circle)
            {
                // Circle-Circle collision
                float distance = Utils::Math::distance(thisPos, otherPos);
                return distance < (radius + other->radius);
            }
            else if (shape == ColliderShape::Rectangle && other->shape == ColliderShape::Rectangle)
            {
                // Rectangle-Rectangle collision (AABB)
                sf::Vector2f thisTopLeft = thisPos - size / 2.f;
                sf::Vector2f otherTopLeft = otherPos - other->size / 2.f;
                sf::FloatRect thisRect(thisTopLeft, size);
                sf::FloatRect otherRect(otherTopLeft, other->size);
                return thisRect.findIntersection(otherRect).has_value();
            }
            else
            {
                // Circle-Rectangle collision
                const Collider* circle = (shape == ColliderShape::Circle) ? this : other;
                const Collider* rect = (shape == ColliderShape::Rectangle) ? this : other;
                sf::Vector2f circlePos = (shape == ColliderShape::Circle) ? thisPos : otherPos;
                sf::Vector2f rectPos = (shape == ColliderShape::Rectangle) ? thisPos : otherPos;

                return circleRectIntersect(circlePos, circle->radius, rectPos, rect->size);
            }
        }

        sf::FloatRect getBounds(const sf::Vector2f& position) const
        {
            if (shape == ColliderShape::Circle)
            {
                return sf::FloatRect(
                    sf::Vector2f(position.x - radius, position.y - radius),
                    sf::Vector2f(radius * 2.f, radius * 2.f)
                );
            }
            else
            {
                return sf::FloatRect(position - size / 2.f, size);
            }
        }

        ColliderShape shape;
        float radius;
        sf::Vector2f size;
        uint32_t layer;
        uint32_t mask;
        bool isTrigger; // If true, collision detected but no physics response

        std::function<void(Entity*)> onCollisionEnter;
        std::function<void(Entity*)> onCollisionExit;

    private:
        bool circleRectIntersect(const sf::Vector2f& circlePos, float circleRadius,
                                const sf::Vector2f& rectPos, const sf::Vector2f& rectSize) const
        {
            // Find closest point on rectangle to circle
            sf::FloatRect rect(rectPos - rectSize / 2.f, rectSize);

            float closestX = std::max(rect.position.x, std::min(circlePos.x, rect.position.x + rect.size.x));
            float closestY = std::max(rect.position.y, std::min(circlePos.y, rect.position.y + rect.size.y));

            float distanceX = circlePos.x - closestX;
            float distanceY = circlePos.y - closestY;

            float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
            return distanceSquared < (circleRadius * circleRadius);
        }
    };
}
