#pragma once
#include "../Component.h"
#include <functional>
#include <algorithm>

namespace MediocreBONK::ECS::Components
{
    class Health : public Component
    {
    public:
        Health(float maxHealth)
            : maxHealth(maxHealth)
            , currentHealth(maxHealth)
            , invulnerable(false)
            , onDamageCallback(nullptr)
            , onDeathCallback(nullptr)
            , onHealCallback(nullptr)
        {}

        void takeDamage(float damage)
        {
            if (invulnerable || currentHealth <= 0.f)
                return;

            currentHealth -= damage;
            currentHealth = std::max(0.f, currentHealth);

            if (onDamageCallback)
                onDamageCallback(damage);

            if (currentHealth <= 0.f && onDeathCallback)
                onDeathCallback();
        }

        void heal(float amount)
        {
            if (currentHealth <= 0.f)
                return;

            currentHealth += amount;
            currentHealth = std::min(currentHealth, maxHealth);

            if (onHealCallback)
                onHealCallback(amount);
        }

        void setMaxHealth(float newMax)
        {
            float ratio = currentHealth / maxHealth;
            maxHealth = newMax;
            currentHealth = maxHealth * ratio;
        }

        bool isAlive() const
        {
            return currentHealth > 0.f;
        }

        bool isDead() const
        {
            return currentHealth <= 0.f;
        }

        float getHealthPercentage() const
        {
            return currentHealth / maxHealth;
        }

        float currentHealth;
        float maxHealth;
        bool invulnerable;

        // Callbacks
        std::function<void(float)> onDamageCallback;
        std::function<void()> onDeathCallback;
        std::function<void(float)> onHealCallback;
    };
}
