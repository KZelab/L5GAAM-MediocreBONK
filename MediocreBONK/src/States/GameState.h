#pragma once
#include "State.h"
#include "../ECS/EntityManager.h"
#include "../Entities/Player.h"
#include "../Entities/Enemy.h"
#include "../Managers/CameraManager.h"
#include "../Managers/DifficultyManager.h"
#include "../Managers/EventManager.h"
#include "../Managers/AudioManager.h"
#include "../Systems/WeaponSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/SpawnSystem.h"
#include "../Systems/XPSystem.h"
#include "../Systems/PowerUpSystem.h"
#include "../Systems/ParticleSystem.h"
#include "../Systems/WorldGenerator.h"
#include "../UI/HUD.h"
#include "../UI/LevelUpMenu.h"
#include "../UI/NotificationManager.h"
#include "../Managers/UpgradeManager.h"
#include "../Utils/Logger.h"
#include "../Utils/Profiler.h"
#include <memory>

// Forward declarations to avoid circular dependencies
namespace MediocreBONK::States
{
    class DeathState;
    class MenuState;
}

namespace MediocreBONK::States
{
    class GameState : public State
    {
    public:
        GameState()
            : entityManager(std::make_unique<ECS::EntityManager>())
            , weaponSystem(nullptr)
            , collisionSystem(nullptr)
            , spawnSystem(nullptr)
            , xpSystem(nullptr)
            , powerUpSystem(nullptr)
            , particleSystem(nullptr)
            , worldGenerator(std::make_unique<Systems::WorldGenerator>(1000.f))
            , hud(nullptr)
            , levelUpMenu(std::make_unique<UI::LevelUpMenu>())
            , player(nullptr)
        {}

        void enter() override
        {
            Utils::Logger::info("Entered Game State");

            // Initialize camera
            Managers::CameraManager::getInstance().initialize(sf::Vector2u(1920, 1080));

            // Calculate screen-relative spawn/despawn distances
            float viewHalfDiagonal = Managers::CameraManager::getInstance().getViewHalfDiagonal();
            const float OFFSCREEN_BUFFER = 50.f; // Spawn 50 pixels off-screen
            const float DESPAWN_MULTIPLIER = 1.5f; // Despawn at 1.5x view diagonal

            float spawnRadius = viewHalfDiagonal + OFFSCREEN_BUFFER;
            float despawnDistance = viewHalfDiagonal * DESPAWN_MULTIPLIER;

            // Log for verification
            Utils::Logger::info("Spawn radius: " + std::to_string(spawnRadius) +
                               " | Despawn distance: " + std::to_string(despawnDistance));

            // Initialize difficulty manager
            Managers::DifficultyManager::getInstance().initialize();

            // Initialize audio manager
            Managers::AudioManager::getInstance().initialize();
            // Note: Sound/music files would be loaded here once assets are sourced

            // Create player entity
            auto* playerEntity = entityManager->createEntity();
            player = std::make_unique<Entities::Player>(playerEntity, sf::Vector2f(960.f, 300.f));

            // Initialize systems (needs player to be created first)
            weaponSystem = std::make_unique<Systems::WeaponSystem>(entityManager.get());
            collisionSystem = std::make_unique<Systems::CollisionSystem>(entityManager.get());
            spawnSystem = std::make_unique<Systems::SpawnSystem>(
                entityManager.get(), player->getEntity(), spawnRadius, despawnDistance);
            xpSystem = std::make_unique<Systems::XPSystem>(entityManager.get(), player->getEntity());
            powerUpSystem = std::make_unique<Systems::PowerUpSystem>(entityManager.get(), player->getEntity());
            particleSystem = std::make_unique<Systems::ParticleSystem>(entityManager.get());

            // Connect spawn system to XP system (enemies drop XP on death)
            spawnSystem->setOnEnemyDeathXPCallback([this](sf::Vector2f pos, float value) {
                xpSystem->spawnXPGem(pos, value);
                hud->incrementKillCount(); // Increment kill count when enemy dies
            });

            // Initialize HUD
            hud = std::make_unique<UI::HUD>(player->getEntity());

            // Initialize notification manager
            notificationManager = std::make_unique<UI::NotificationManager>();
            notificationManager->initialize();

            // Wire particle system to buff events
            Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::BuffApplied,
                [this](const Managers::EventData* data) {
                    if (auto* buffData = dynamic_cast<const Managers::BuffAppliedData*>(data))
                    {
                        auto* playerTransform = player->getEntity()->getComponent<ECS::Components::Transform>();
                        if (playerTransform)
                        {
                            // Determine color based on buff name
                            sf::Color color = sf::Color::White;
                            if (buffData->buffName.find("Damage") != std::string::npos)
                                color = sf::Color(255, 100, 100);  // Red
                            else if (buffData->buffName.find("Speed") != std::string::npos)
                                color = sf::Color(100, 255, 255);  // Cyan
                            else if (buffData->buffName.find("Invulnerability") != std::string::npos)
                                color = sf::Color(255, 255, 100);  // Yellow
                            else if (buffData->buffName.find("XP") != std::string::npos)
                                color = sf::Color(255, 100, 255);  // Magenta
                            else if (buffData->buffName.find("Health") != std::string::npos || buffData->buffName.find("Regen") != std::string::npos)
                                color = sf::Color(100, 255, 100);  // Green
                            else if (buffData->buffName.find("Fire") != std::string::npos)
                                color = sf::Color(255, 165, 0);    // Orange

                            particleSystem->spawnBuffApplied(playerTransform->position, color);
                        }
                    }
                }
            );

            Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::BuffExpired,
                [this](const Managers::EventData* data) {
                    if (auto* buffData = dynamic_cast<const Managers::BuffAppliedData*>(data))
                    {
                        auto* playerTransform = player->getEntity()->getComponent<ECS::Components::Transform>();
                        if (playerTransform)
                        {
                            particleSystem->spawnBuffExpired(playerTransform->position);
                        }
                    }
                }
            );

            Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::PlayerLevelUp,
                [this](const Managers::EventData* data) {
                    if (auto* levelUpData = dynamic_cast<const Managers::PlayerLevelUpData*>(data))
                    {
                        auto* playerTransform = player->getEntity()->getComponent<ECS::Components::Transform>();
                        if (playerTransform)
                        {
                            particleSystem->spawnLevelUp(playerTransform->position);
                        }
                    }
                }
            );

            Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::PowerUpCollected,
                [this](const Managers::EventData* data) {
                    if (auto* powerUpData = dynamic_cast<const Managers::BuffAppliedData*>(data))
                    {
                        auto* playerTransform = player->getEntity()->getComponent<ECS::Components::Transform>();
                        if (playerTransform)
                        {
                            particleSystem->spawnPickupEffect(playerTransform->position);
                        }
                    }
                }
            );

            // Initialize upgrade manager
            Managers::UpgradeManager::getInstance().initialize();

            // Set camera to follow player
            Managers::CameraManager::getInstance().setFollowTarget(player->getEntity());

            Utils::Logger::info("Player created, systems initialized, and camera set");
        }

        void exit() override
        {
            Utils::Logger::info("Exited Game State");
            entityManager->clear();
        }

        void update(sf::Time dt) override
        {
            // Check if player is dead
            if (player)
            {
                auto* health = player->getEntity()->getComponent<ECS::Components::Health>();
                if (health && health->currentHealth <= 0.f)
                {
                    // Player died - transition to death state
                    auto* experience = player->getEntity()->getComponent<ECS::Components::Experience>();
                    int playerLevel = experience ? experience->getCurrentLevel() : 1;

                    Utils::Logger::info("Player died! Transitioning to Death State");
                    transitionToDeathState(hud->getGameTime(), hud->getKillCount(), playerLevel);
                    return;
                }
            }

            // Check if level-up menu should be shown
            if (player && player->hasLevelUpPending() && !levelUpMenu->getIsVisible())
            {
                levelUpMenu->show(player->getEntity());
                player->clearLevelUpPending();
            }

            // If level-up menu is open, don't update game
            if (levelUpMenu->getIsVisible())
            {
                return;
            }

            // Update difficulty (scales enemy stats over time)
            Managers::DifficultyManager::getInstance().update(dt);
            Managers::DifficultyManager::getInstance().applyToSpawnSystem(spawnSystem.get());

            // Update player
            if (player)
            {
                player->handleInput();
                player->update(dt);

                // Update world generation based on player position
                auto* playerTransform = player->getEntity()->getComponent<ECS::Components::Transform>();
                if (playerTransform)
                {
                    worldGenerator->update(playerTransform->position);
                }
            }

            // Update all entities
            Utils::Profiler::start("Entities Update");
            entityManager->update(dt);
            Utils::Profiler::stop("Entities Update");

            // Update systems
            Utils::Profiler::start("WeaponSystem");
            weaponSystem->update(dt);
            Utils::Profiler::stop("WeaponSystem");

            Utils::Profiler::start("CollisionSystem");
            collisionSystem->update(dt);
            Utils::Profiler::stop("CollisionSystem");

            Utils::Profiler::start("SpawnSystem");
            spawnSystem->update(dt);
            Utils::Profiler::stop("SpawnSystem");

            xpSystem->update(dt);
            powerUpSystem->update(dt);
            particleSystem->update(dt);

            // Process queued events
            Managers::EventManager::getInstance().processEvents();

            // Performance monitoring (every 5 seconds)
            static float perfTimer = 0.f;
            perfTimer += dt.asSeconds();
            if (perfTimer >= 5.f)
            {
                perfTimer = 0.f;
                size_t totalEntities = entityManager->getTotalEntityCount();
                size_t activeEntities = entityManager->getEntityCount();
                auto enemies = entityManager->getEntitiesByTag("Enemy");
                auto projectiles = entityManager->getEntitiesWithComponent<ECS::Components::Projectile>();
                Utils::Logger::info("Performance: Total=" + std::to_string(totalEntities) +
                                   " Active=" + std::to_string(activeEntities) +
                                   " Enemies=" + std::to_string(enemies.size()) +
                                   " Projectiles=" + std::to_string(projectiles.size()));
                
                Utils::Profiler::logResults();
            }

            // Update HUD
            hud->update(dt);

            // Update notification manager
            if (notificationManager)
                notificationManager->update(dt);

            // Update camera
            Managers::CameraManager::getInstance().update(dt);
        }

        void render(sf::RenderWindow& window) override
        {
            // Set game view for world rendering
            window.setView(Managers::CameraManager::getInstance().getGameView());

            // Render infinite tiling world
            worldGenerator->render(window);

            // Draw player placeholder (since we don't have sprite yet)
            if (player)
            {
                auto* transform = player->getEntity()->getComponent<ECS::Components::Transform>();
                if (transform)
                {
                    sf::CircleShape playerShape(20.f);
                    playerShape.setOrigin({20.f, 20.f});
                    playerShape.setPosition(transform->position);
                    playerShape.setFillColor(sf::Color::Green);
                    window.draw(playerShape);
                }
            }

            // Draw enemies
            auto enemies = entityManager->getEntitiesByTag("Enemy");

            static bool loggedEnemyCount = false;
            static int lastEnemyCount = 0;
            if (!loggedEnemyCount || enemies.size() != lastEnemyCount)
            {
                Utils::Logger::info("Rendering " + std::to_string(enemies.size()) + " enemies");
                lastEnemyCount = enemies.size();
                loggedEnemyCount = true;
            }

            for (auto* enemy : enemies)
            {
                auto* transform = enemy->getComponent<ECS::Components::Transform>();
                auto* collider = enemy->getComponent<ECS::Components::Collider>();
                auto* health = enemy->getComponent<ECS::Components::Health>();

                if (transform && collider)
                {
                    sf::CircleShape enemyShape(collider->radius);
                    enemyShape.setOrigin({collider->radius, collider->radius});
                    enemyShape.setPosition(transform->position);

                    // Color based on health percentage
                    float healthPercent = health ? health->getHealthPercentage() : 1.f;
                    sf::Color baseColor = sf::Color::Red;
                    sf::Color color(
                        static_cast<std::uint8_t>(baseColor.r * healthPercent),
                        static_cast<std::uint8_t>(baseColor.g * healthPercent),
                        static_cast<std::uint8_t>(baseColor.b * healthPercent)
                    );

                    enemyShape.setFillColor(color);
                    window.draw(enemyShape);
                }
            }

            // Draw projectiles
            auto projectiles = entityManager->getEntitiesWithComponent<ECS::Components::Projectile>();

            static bool loggedProjectileCount = false;
            static int lastProjectileCount = 0;
            if (!loggedProjectileCount || projectiles.size() != lastProjectileCount)
            {
                Utils::Logger::info("Rendering " + std::to_string(projectiles.size()) + " projectiles");
                lastProjectileCount = projectiles.size();
                loggedProjectileCount = true;
            }

            for (auto* proj : projectiles)
            {
                auto* transform = proj->getComponent<ECS::Components::Transform>();
                if (transform)
                {
                    sf::CircleShape projShape(5.f);
                    projShape.setOrigin({5.f, 5.f});
                    projShape.setPosition(transform->position);
                    projShape.setFillColor(sf::Color::Yellow);
                    window.draw(projShape);
                }
            }

            // Draw XP gems
            auto xpGems = entityManager->getEntitiesByTag("XPGem");
            for (auto* gem : xpGems)
            {
                auto* transform = gem->getComponent<ECS::Components::Transform>();
                if (transform)
                {
                    sf::CircleShape gemShape(8.f);
                    gemShape.setOrigin({8.f, 8.f});
                    gemShape.setPosition(transform->position);
                    gemShape.setFillColor(sf::Color::Cyan); // Bright cyan for XP
                    window.draw(gemShape);
                }
            }

            // Draw power-ups
            auto powerUps = entityManager->getEntitiesByTag("PowerUp");
            for (auto* powerUp : powerUps)
            {
                auto* transform = powerUp->getComponent<ECS::Components::Transform>();
                auto* collider = powerUp->getComponent<ECS::Components::Collider>();
                if (transform && collider)
                {
                    // Draw power-up as a diamond shape
                    sf::CircleShape powerUpShape(collider->radius, 4); // 4 points = diamond
                    powerUpShape.setOrigin({collider->radius, collider->radius});
                    powerUpShape.setPosition(transform->position);
                    powerUpShape.setRotation(sf::degrees(45.f)); // Rotate to make diamond orientation

                    // Color will need to come from PowerUp data in full implementation
                    powerUpShape.setFillColor(sf::Color::Magenta);
                    powerUpShape.setOutlineThickness(2.f);
                    powerUpShape.setOutlineColor(sf::Color::White);
                    window.draw(powerUpShape);
                }
            }

            // Render all entities
            Utils::Profiler::start("Render Entities");
            entityManager->render(window);
            Utils::Profiler::stop("Render Entities");

            // Render particles
            particleSystem->render(window);

            // Set UI view for HUD
            window.setView(Managers::CameraManager::getInstance().getUIView());

            // Draw HUD
            hud->render(window);

            // Draw notifications on top of HUD
            if (notificationManager)
                notificationManager->render(window);

            // Draw level-up menu on top of everything
            levelUpMenu->render(window);
        }

        void handleInput(const sf::Event& event) override
        {
            // If level-up menu is open, route input to it
            if (levelUpMenu->getIsVisible())
            {
                levelUpMenu->handleInput(event);
                return;
            }

            // Handle input
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::LShift ||
                    keyPressed->code == sf::Keyboard::Key::RShift)
                {
                    if (player)
                        player->dash();
                }

                // Return to menu
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    stateMachine->popState();
                }
            }
        }

    private:
        void transitionToDeathState(float survivalTime, int killCount, int level);

        std::unique_ptr<ECS::EntityManager> entityManager;
        std::unique_ptr<Systems::WeaponSystem> weaponSystem;
        std::unique_ptr<Systems::CollisionSystem> collisionSystem;
        std::unique_ptr<Systems::SpawnSystem> spawnSystem;
        std::unique_ptr<Systems::XPSystem> xpSystem;
        std::unique_ptr<Systems::PowerUpSystem> powerUpSystem;
        std::unique_ptr<Systems::ParticleSystem> particleSystem;
        std::unique_ptr<Systems::WorldGenerator> worldGenerator;
        std::unique_ptr<UI::HUD> hud;
        std::unique_ptr<UI::LevelUpMenu> levelUpMenu;
        std::unique_ptr<UI::NotificationManager> notificationManager;
        std::unique_ptr<Entities::Player> player;
    };
}

// Include DeathState here to avoid circular dependency issues
#include "DeathState.h"

namespace MediocreBONK::States
{
    inline void GameState::transitionToDeathState(float survivalTime, int killCount, int level)
    {
        stateMachine->changeState(std::make_unique<DeathState>(survivalTime, killCount, level));
    }
}
