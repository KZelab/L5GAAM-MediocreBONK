#pragma once
#include "../ECS/Entity.h"
#include "../ECS/Components/Weapon.h"
#include "../ECS/Components/Health.h"
#include "../ECS/Components/Physics.h"
#include "../Utils/Random.h"
#include <vector>
#include <string>
#include <functional>

namespace MediocreBONK::Managers
{
    enum class UpgradeType
    {
        DamageIncrease,
        FireRateIncrease,
        ProjectileCountIncrease,
        HealthIncrease,
        SpeedIncrease,
        PiercingIncrease
    };

    struct Upgrade
    {
        std::string name;
        std::string description;
        UpgradeType type;
        int currentLevel;
        int maxLevel;
        std::function<void(ECS::Entity*)> applyEffect;

        bool isMaxed() const { return currentLevel >= maxLevel; }
    };

    class UpgradeManager
    {
    public:
        static UpgradeManager& getInstance()
        {
            static UpgradeManager instance;
            return instance;
        }

        UpgradeManager(const UpgradeManager&) = delete;
        UpgradeManager& operator=(const UpgradeManager&) = delete;

        void initialize()
        {
            createUpgrades();
        }

        std::vector<Upgrade*> getRandomUpgrades(int count = 3)
        {
            std::vector<Upgrade*> available;

            // Get all non-maxed upgrades
            for (auto& upgrade : upgrades)
            {
                if (!upgrade.isMaxed())
                {
                    available.push_back(&upgrade);
                }
            }

            // Shuffle and pick random ones
            std::vector<Upgrade*> selected;
            int numToSelect = std::min(count, static_cast<int>(available.size()));

            for (int i = 0; i < numToSelect; ++i)
            {
                int randomIndex = Utils::Random::range(0, static_cast<int>(available.size()) - 1);
                selected.push_back(available[randomIndex]);
                available.erase(available.begin() + randomIndex);
            }

            return selected;
        }

        void applyUpgrade(Upgrade* upgrade, ECS::Entity* player)
        {
            if (upgrade && !upgrade->isMaxed())
            {
                upgrade->applyEffect(player);
                upgrade->currentLevel++;
            }
        }

        void reset()
        {
            for (auto& upgrade : upgrades)
            {
                upgrade.currentLevel = 0;
            }
        }

    private:
        UpgradeManager() = default;

        void createUpgrades()
        {
            // Damage increase
            upgrades.push_back({
                "Damage Boost",
                "+5 Damage",
                UpgradeType::DamageIncrease,
                0, 10, // Max level 10
                [](ECS::Entity* player) {
                    auto* weapon = player->getComponent<ECS::Components::Weapon>();
                    if (weapon) weapon->upgradeDamage(5.f);
                }
            });

            // Fire rate increase
            upgrades.push_back({
                "Faster Shooting",
                "+1 Fire Rate",
                UpgradeType::FireRateIncrease,
                0, 10,
                [](ECS::Entity* player) {
                    auto* weapon = player->getComponent<ECS::Components::Weapon>();
                    if (weapon) weapon->upgradeFireRate(1.f);
                }
            });

            // Projectile count
            upgrades.push_back({
                "Multi-Shot",
                "+1 Projectile",
                UpgradeType::ProjectileCountIncrease,
                0, 5,
                [](ECS::Entity* player) {
                    auto* weapon = player->getComponent<ECS::Components::Weapon>();
                    if (weapon) weapon->upgradeProjectileCount(1);
                }
            });

            // Health increase
            upgrades.push_back({
                "Max Health Up",
                "+20 Max Health",
                UpgradeType::HealthIncrease,
                0, 5,
                [](ECS::Entity* player) {
                    auto* health = player->getComponent<ECS::Components::Health>();
                    if (health) {
                        health->setMaxHealth(health->maxHealth + 20.f);
                        health->heal(20.f); // Also heal when upgrading
                    }
                }
            });

            // Speed increase
            upgrades.push_back({
                "Speed Boost",
                "+10% Move Speed",
                UpgradeType::SpeedIncrease,
                0, 5,
                [](ECS::Entity* player) {
                    auto* physics = player->getComponent<ECS::Components::Physics>();
                    if (physics) {
                        physics->maxSpeed *= 1.1f;
                    }
                }
            });

            // Piercing increase
            upgrades.push_back({
                "Piercing Shot",
                "+1 Piercing",
                UpgradeType::PiercingIncrease,
                0, 5,
                [](ECS::Entity* player) {
                    auto* weapon = player->getComponent<ECS::Components::Weapon>();
                    if (weapon) weapon->upgradePiercing(1);
                }
            });
        }

        std::vector<Upgrade> upgrades;
    };
}
