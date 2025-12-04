#pragma once
#include "../Component.h"
#include "../../Managers/EventManager.h"
#include <string>
#include <vector>
#include <functional>
#include <SFML/System/Time.hpp>

namespace MediocreBONK::ECS::Components
{
    enum class BuffType
    {
        DamageBoost,
        SpeedBoost,
        InvulnerabilityBoost,
        FireRateBoost,
        HealthRegen,
        MagnetRange,
        XPMultiplier
    };

    struct BuffEffect
    {
        std::string name;
        BuffType type;
        float value;               // Multiplier or additive value
        float duration;            // Total duration in seconds (-1 for permanent)
        float remainingTime;       // Time remaining
        bool isPermanent;
        std::function<void()> onApply;   // Called when buff is applied
        std::function<void()> onExpire;  // Called when buff expires

        BuffEffect(const std::string& name, BuffType type, float value, float duration = -1.f)
            : name(name)
            , type(type)
            , value(value)
            , duration(duration)
            , remainingTime(duration)
            , isPermanent(duration < 0)
            , onApply(nullptr)
            , onExpire(nullptr)
        {}

        bool hasExpired() const
        {
            return !isPermanent && remainingTime <= 0.f;
        }

        void update(float dt)
        {
            if (!isPermanent)
            {
                remainingTime -= dt;
            }
        }
    };

    class Buff : public Component
    {
    public:
        Buff() = default;

        void update(sf::Time dt) override
        {
            float deltaTime = dt.asSeconds();

            // Update all buffs
            for (auto& buff : activeBuffs)
            {
                buff.update(deltaTime);
            }

            // Remove expired buffs
            activeBuffs.erase(
                std::remove_if(activeBuffs.begin(), activeBuffs.end(),
                    [](const BuffEffect& buff) {
                        if (buff.hasExpired())
                        {
                            // Call expire callback
                            if (buff.onExpire)
                                buff.onExpire();

                            // Emit BuffExpired event
                            auto eventData = std::make_unique<Managers::BuffAppliedData>();
                            eventData->buffName = buff.name;
                            eventData->duration = 0.f;
                            Managers::EventManager::getInstance().queueEvent(
                                Managers::GameEventType::BuffExpired, std::move(eventData));

                            return true;
                        }
                        return false;
                    }),
                activeBuffs.end()
            );
        }

        void addBuff(const BuffEffect& buff)
        {
            // Check if buff already exists and refresh it
            for (auto& existingBuff : activeBuffs)
            {
                if (existingBuff.name == buff.name)
                {
                    existingBuff.remainingTime = buff.duration;
                    return;
                }
            }

            // Add new buff
            activeBuffs.push_back(buff);
            if (buff.onApply)
                buff.onApply();

            // Emit BuffApplied event
            auto eventData = std::make_unique<Managers::BuffAppliedData>();
            eventData->buffName = buff.name;
            eventData->duration = buff.duration;
            Managers::EventManager::getInstance().queueEvent(
                Managers::GameEventType::BuffApplied, std::move(eventData));
        }

        void removeBuff(const std::string& buffName)
        {
            activeBuffs.erase(
                std::remove_if(activeBuffs.begin(), activeBuffs.end(),
                    [&buffName](const BuffEffect& buff) {
                        if (buff.name == buffName)
                        {
                            if (buff.onExpire)
                                buff.onExpire();
                            return true;
                        }
                        return false;
                    }),
                activeBuffs.end()
            );
        }

        bool hasBuff(const std::string& buffName) const
        {
            return std::any_of(activeBuffs.begin(), activeBuffs.end(),
                [&buffName](const BuffEffect& buff) {
                    return buff.name == buffName;
                });
        }

        float getBuffValue(BuffType type) const
        {
            float totalValue = 0.f;
            for (const auto& buff : activeBuffs)
            {
                if (buff.type == type)
                    totalValue += buff.value;
            }
            return totalValue;
        }

        // Get multiplier for a buff type (1.0 base + all buff values)
        float getBuffMultiplier(BuffType type) const
        {
            return 1.0f + getBuffValue(type);
        }

        const std::vector<BuffEffect>& getActiveBuffs() const
        {
            return activeBuffs;
        }

        void clearAllBuffs()
        {
            for (auto& buff : activeBuffs)
            {
                if (buff.onExpire)
                    buff.onExpire();
            }
            activeBuffs.clear();
        }

    private:
        std::vector<BuffEffect> activeBuffs;
    };
}
