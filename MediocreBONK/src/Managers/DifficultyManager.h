#pragma once
#include "../Utils/Logger.h"
#include <SFML/System/Time.hpp>

// Forward declaration to avoid circular dependency
namespace MediocreBONK::Systems
{
    class SpawnSystem;
}

namespace MediocreBONK::Managers
{
    struct DifficultySettings
    {
        float spawnInterval;     // Time between spawns
        int maxEnemies;          // Max concurrent enemies
        float enemyHealthMult;   // Multiplier for enemy health
        float enemySpeedMult;    // Multiplier for enemy speed
        float enemyDamageMult;   // Multiplier for enemy damage
        float xpValueMult;       // Multiplier for XP drops
    };

    class DifficultyManager
    {
    public:
        static DifficultyManager& getInstance()
        {
            static DifficultyManager instance;
            return instance;
        }

        DifficultyManager(const DifficultyManager&) = delete;
        DifficultyManager& operator=(const DifficultyManager&) = delete;

        void initialize()
        {
            gameTime = 0.f;
            currentSettings = getBaseSettings();
            Utils::Logger::info("DifficultyManager initialized");
        }

        void update(sf::Time dt)
        {
            gameTime += dt.asSeconds();
            updateDifficulty();
        }

        void reset()
        {
            gameTime = 0.f;
            currentSettings = getBaseSettings();
        }

        float getGameTime() const { return gameTime; }

        // Difficulty multipliers for enemy stats
        float getHealthMultiplier() const { return currentSettings.enemyHealthMult; }
        float getSpeedMultiplier() const { return currentSettings.enemySpeedMult; }
        float getDamageMultiplier() const { return currentSettings.enemyDamageMult; }
        float getXPMultiplier() const { return currentSettings.xpValueMult; }

        // Apply current difficulty to spawn system
        void applyToSpawnSystem(Systems::SpawnSystem* spawnSystem);

    private:
        DifficultyManager()
            : gameTime(0.f)
            , currentSettings(getBaseSettings())
        {}

        DifficultySettings getBaseSettings()
        {
            return DifficultySettings{
                2.5f,   // spawnInterval 
                120,    // maxEnemies (more enemies allowed)
                1.5f,   // enemyHealthMult (30% more health to start)
                1.2f,   // enemySpeedMult (10% faster to start)
                2.0f,   // enemyDamageMult (20% more damage to start)
                1.0f    // xpValueMult
            };
        }

        void updateDifficulty()
        {
            // Difficulty scales based on time milestones
            // Every 30 seconds, difficulty increases

            int difficultyLevel = static_cast<int>(gameTime / 30.f);

            // More aggressive scaling: +50% per level instead of 15%
            float healthScaleFactor = 1.3f + (difficultyLevel * 0.50f);
            float damageScaleFactor = 1.2f + (difficultyLevel * 0.50f);

            // Speed scaling: starts at 1.1x, increases by 16% per level (much faster progression)
            float speedScaleFactor = 1.1f + (difficultyLevel * 0.16f);

            // Update spawn parameters - enemies spawn faster and in greater numbers
            currentSettings.spawnInterval = std::max(0.3f, 1.5f - (difficultyLevel * 0.12f));

            // Update enemy stat multipliers
            currentSettings.enemyHealthMult = healthScaleFactor;
            currentSettings.enemySpeedMult = speedScaleFactor;
            currentSettings.enemyDamageMult = damageScaleFactor;

            // XP scales with difficulty (slightly less than health/damage to maintain challenge)
            currentSettings.xpValueMult = 1.0f + (difficultyLevel * 0.12f);
        }

        float gameTime;
        DifficultySettings currentSettings;
    };
}

// Include SpawnSystem here to avoid circular dependency
#include "../Systems/SpawnSystem.h"

namespace MediocreBONK::Managers
{
    inline void DifficultyManager::applyToSpawnSystem(Systems::SpawnSystem* spawnSystem)
    {
        if (spawnSystem)
        {
            spawnSystem->setSpawnInterval(currentSettings.spawnInterval);
            spawnSystem->setMaxEnemies(currentSettings.maxEnemies);
        }
    }
}
