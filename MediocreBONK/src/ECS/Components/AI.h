#pragma once
#include "../Component.h"
#include "Transform.h"
#include "Physics.h"
#include "../../Utils/Math.h"
#include <SFML/System/Vector2.hpp>

namespace MediocreBONK::ECS::Components
{
    enum class AIBehavior
    {
        ChasePlayer,
        Flee,
        Ranged,
        Circle,
        Idle
    };

    class AI : public Component
    {
    public:
        AI(AIBehavior behavior = AIBehavior::ChasePlayer, float speed = 100.f)
            : behavior(behavior)
            , speed(speed)
            , target(nullptr)
            , targetPosition(0.f, 0.f)
            , attackRange(50.f)
            , detectionRange(1500.f) // Increased to detect player from spawn distance (~1151px)
        {}

        void update(sf::Time dt) override
        {
            if (!owner || !target)
                return;

            auto* transform = owner->getComponent<Transform>();
            auto* physics = owner->getComponent<Physics>();

            if (!transform || !physics)
                return;

            auto* targetTransform = target->getComponent<Transform>();
            if (!targetTransform)
                return;

            targetPosition = targetTransform->position;
            float distance = Utils::Math::distance(transform->position, targetPosition);

            // Check if target is in detection range
            if (distance > detectionRange)
                return;

            switch (behavior)
            {
            case AIBehavior::ChasePlayer:
                chaseTarget(transform, physics, distance);
                break;
            case AIBehavior::Flee:
                fleeFromTarget(transform, physics);
                break;
            case AIBehavior::Idle:
                // Do nothing
                break;
            // Future behaviors can be added here
            default:
                break;
            }
        }

        void setTarget(Entity* newTarget)
        {
            target = newTarget;
        }

        AIBehavior behavior;
        float speed;
        float attackRange;
        float detectionRange;

    private:
        void chaseTarget(Transform* transform, Physics* physics, float distance)
        {
            // Stop if within attack range
            if (distance < attackRange)
            {
                physics->velocity = sf::Vector2f(0.f, 0.f);
                return;
            }

            // Calculate direction to target
            sf::Vector2f direction = Utils::Math::normalize(targetPosition - transform->position);

            // Apply movement force
            physics->applyForce(direction * speed);
        }

        void fleeFromTarget(Transform* transform, Physics* physics)
        {
            // Move away from target
            sf::Vector2f direction = Utils::Math::normalize(transform->position - targetPosition);
            physics->applyForce(direction * speed);
        }

        Entity* target;
        sf::Vector2f targetPosition;
    };
}
