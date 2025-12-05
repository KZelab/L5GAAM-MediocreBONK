#pragma once
#include "../Component.h"
#include "../../Managers/EventManager.h"
#include <functional>
#include <cmath>

namespace MediocreBONK::ECS::Components
{
    class Experience : public Component
    {
    public:
        Experience(int startLevel = 1)
            : currentLevel(startLevel)
            , currentXP(0.f)
            , xpToNextLevel(calculateXPForLevel(startLevel))
        {}

        void addXP(float amount)
        {
            currentXP += amount;

            // Check for level up
            while (currentXP >= xpToNextLevel)
            {
                levelUp();
            }
        }

        void levelUp()
        {
            int previousLevel = currentLevel;
            currentLevel++;
            currentXP -= xpToNextLevel;
            xpToNextLevel = calculateXPForLevel(currentLevel);

            // Emit PlayerLevelUp event
            auto eventData = std::make_unique<Managers::PlayerLevelUpData>();
            eventData->newLevel = currentLevel;
            eventData->previousLevel = previousLevel;
            Managers::EventManager::getInstance().queueEvent(
                Managers::GameEventType::PlayerLevelUp, std::move(eventData));

            // Trigger level up callback
            if (onLevelUpCallback)
                onLevelUpCallback(currentLevel);
        }

        float getXPPercentage() const
        {
            return currentXP / xpToNextLevel;
        }

        int getCurrentLevel() const { return currentLevel; }
        float getCurrentXP() const { return currentXP; }
        float getXPToNextLevel() const { return xpToNextLevel; }

        std::function<void(int)> onLevelUpCallback;

    private:
        float calculateXPForLevel(int level)
        {
            // Exponential XP curve: baseXP * (growthRate ^ level)
            // Balanced for slower early-game progression
            float baseXP = 50.f;       // Increased from 10 (5x harder to level)
            float growthRate = 1.5f;   // Increased from 1.2 (steeper curve)
            return baseXP * std::pow(growthRate, static_cast<float>(level));
        }

        int currentLevel;
        float currentXP;
        float xpToNextLevel;
    };
}
