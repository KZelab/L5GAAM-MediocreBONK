#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components/Transform.h"
#include "../Entities/Enemy.h"
#include "../Utils/Random.h"
#include "../Utils/Logger.h"
#include "../Utils/Math.h"
#include <SFML/System/Time.hpp>
#include <memory>
#include <vector>

namespace MediocreBONK::Systems
{
    class SpawnSystem
    {
    public:
        SpawnSystem(ECS::EntityManager* entityManager, ECS::Entity* player,
                    float spawnRadius, float despawnDistance)
            : entityManager(entityManager)
            , player(player)
            , spawnTimer(0.f)
            , spawnInterval(2.f) // Spawn every 2 seconds
            , spawnRadius(spawnRadius) // Screen-relative spawn distance
            , maxEnemies(50) // Reduced from 100 for better performance
            , despawnDistance(despawnDistance) // Screen-relative despawn distance
            , cullCheckTimer(0.f)
            , cullCheckInterval(1.f) // Check for culling every 1 second
            , onEnemyDeathXPCallback(nullptr)
        {}

        void update(sf::Time dt)
        {
            spawnTimer += dt.asSeconds();

            if (spawnTimer >= spawnInterval)
            {
                spawnTimer = 0.f;
                spawnWave();
            }

            // Periodically cull distant enemies
            cullCheckTimer += dt.asSeconds();
            if (cullCheckTimer >= cullCheckInterval)
            {
                cullCheckTimer = 0.f;
                cullDistantEnemies();
            }
        }

        void setSpawnInterval(float interval)
        {
            spawnInterval = interval;
        }

        void setMaxEnemies(int max)
        {
            maxEnemies = max;
        }

        void spawnEnemy(MediocreBONK::Entities::EnemyType type, const sf::Vector2f& position)
        {
            auto enemy = MediocreBONK::Entities::EnemyFactory::create(entityManager, type, position, player);
            if (enemy)
            {
                // Set XP drop callback
                if (onEnemyDeathXPCallback)
                {
                    enemy->setOnDeathXPCallback(onEnemyDeathXPCallback);
                }

                enemy->onSpawn();
                enemies.push_back(std::move(enemy));
            }
        }

        void setOnEnemyDeathXPCallback(std::function<void(sf::Vector2f, float)> callback)
        {
            onEnemyDeathXPCallback = callback;
        }

    private:
        void cullDistantEnemies()
        {
            auto* playerTransform = player->getComponent<ECS::Components::Transform>();
            if (!playerTransform)
                return;

            auto enemies = entityManager->getEntitiesByTag("Enemy");
            int culledCount = 0;

            for (auto* enemy : enemies)
            {
                auto* enemyTransform = enemy->getComponent<ECS::Components::Transform>();
                if (!enemyTransform)
                    continue;

                // Check distance from player
                float distSquared = Utils::Math::distanceSquared(playerTransform->position, enemyTransform->position);
                float maxDistSquared = despawnDistance * despawnDistance;

                if (distSquared > maxDistSquared)
                {
                    // Deactivate enemy that's too far
                    enemy->setActive(false);
                    culledCount++;
                }
            }

            if (culledCount > 0)
            {
                Utils::Logger::info("Culled " + std::to_string(culledCount) + " distant enemies");
            }
        }
        void spawnWave()
        {
            // Check if we've hit max enemies
            int activeEnemies = entityManager->getEntitiesByTag("Enemy").size();

            static bool loggedFirstSpawn = false;
            if (!loggedFirstSpawn)
            {
                Utils::Logger::info("SpawnSystem: Attempting to spawn wave. Active enemies: " +
                                   std::to_string(activeEnemies) + "/" + std::to_string(maxEnemies));
                loggedFirstSpawn = true;
            }

            if (activeEnemies >= maxEnemies)
                return;

            auto* playerTransform = player->getComponent<ECS::Components::Transform>();
            if (!playerTransform)
            {
                Utils::Logger::info("SpawnSystem: No player transform!");
                return;
            }

            // Spawn 3-5 enemies per wave
            int spawnCount = Utils::Random::range(3, 5);

            static bool loggedSpawnCount = false;
            if (!loggedSpawnCount)
            {
                Utils::Logger::info("SpawnSystem: Spawning " + std::to_string(spawnCount) + " enemies");
                loggedSpawnCount = true;
            }

            for (int i = 0; i < spawnCount; ++i)
            {
                // Random position on circle around player
                sf::Vector2f offset = Utils::Random::onCircle(spawnRadius);
                sf::Vector2f spawnPos = playerTransform->position + offset;

                // Random enemy type (weighted)
                MediocreBONK::Entities::EnemyType type = getRandomEnemyType();

                spawnEnemy(type, spawnPos);
            }

            static bool loggedAfterSpawn = false;
            if (!loggedAfterSpawn)
            {
                int newEnemyCount = entityManager->getEntitiesByTag("Enemy").size();
                Utils::Logger::info("SpawnSystem: After spawn, enemy count: " + std::to_string(newEnemyCount));
                loggedAfterSpawn = true;
            }
        }

        MediocreBONK::Entities::EnemyType getRandomEnemyType()
        {
            // Weighted spawn: 60% Light, 30% Medium, 10% Heavy
            float roll = Utils::Random::value();

            if (roll < 0.6f)
                return MediocreBONK::Entities::EnemyType::Light;
            else if (roll < 0.9f)
                return MediocreBONK::Entities::EnemyType::Medium;
            else
                return MediocreBONK::Entities::EnemyType::Heavy;
        }

        ECS::EntityManager* entityManager;
        ECS::Entity* player;
        std::vector<std::unique_ptr<MediocreBONK::Entities::Enemy>> enemies;

        float spawnTimer;
        float spawnInterval;
        float spawnRadius;
        int maxEnemies;
        float despawnDistance;
        float cullCheckTimer;
        float cullCheckInterval;

        std::function<void(sf::Vector2f, float)> onEnemyDeathXPCallback;
    };
}
