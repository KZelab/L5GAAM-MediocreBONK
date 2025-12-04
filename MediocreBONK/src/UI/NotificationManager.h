#pragma once
#include "../Managers/EventManager.h"
#include "../Core/ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace MediocreBONK::UI
{
    class NotificationManager
    {
    public:
        NotificationManager()
            : font(Core::ResourceManager::getInstance().getFont("assets/fonts/arial.ttf"))
        {
        }

        ~NotificationManager()
        {
            cleanup();
        }

        void initialize()
        {
            // Subscribe to events
            int powerUpId = Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::PowerUpCollected,
                [this](const Managers::EventData* data) {
                    if (auto* powerUpData = dynamic_cast<const Managers::BuffAppliedData*>(data))
                    {
                        showNotification(powerUpData->buffName + " Collected!", sf::Color::White, 2.0f);
                    }
                }
            );
            listenerIds.push_back({Managers::GameEventType::PowerUpCollected, powerUpId});

            int buffAppliedId = Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::BuffApplied,
                [this](const Managers::EventData* data) {
                    if (auto* buffData = dynamic_cast<const Managers::BuffAppliedData*>(data))
                    {
                        sf::Color color = getColorForBuff(buffData->buffName);
                        showNotification(buffData->buffName + " Active!", color, 2.5f);
                    }
                }
            );
            listenerIds.push_back({Managers::GameEventType::BuffApplied, buffAppliedId});

            int buffExpiredId = Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::BuffExpired,
                [this](const Managers::EventData* data) {
                    if (auto* buffData = dynamic_cast<const Managers::BuffAppliedData*>(data))
                    {
                        showNotification(buffData->buffName + " Expired", sf::Color(150, 150, 150), 1.5f);
                    }
                }
            );
            listenerIds.push_back({Managers::GameEventType::BuffExpired, buffExpiredId});

            int levelUpId = Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::PlayerLevelUp,
                [this](const Managers::EventData* data) {
                    if (auto* levelUpData = dynamic_cast<const Managers::PlayerLevelUpData*>(data))
                    {
                        showNotification("Level Up! Level " + std::to_string(levelUpData->newLevel),
                                       sf::Color(255, 215, 0), 3.0f);
                    }
                }
            );
            listenerIds.push_back({Managers::GameEventType::PlayerLevelUp, levelUpId});

            int waveCompletedId = Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::WaveCompleted,
                [this](const Managers::EventData* data) {
                    // Future: Add wave number from event data
                    showNotification("Wave Complete!", sf::Color::Green, 3.0f);
                }
            );
            listenerIds.push_back({Managers::GameEventType::WaveCompleted, waveCompletedId});

            int bossSpawnedId = Managers::EventManager::getInstance().subscribe(
                Managers::GameEventType::BossSpawned,
                [this](const Managers::EventData* data) {
                    showNotification("Boss Incoming!", sf::Color::Red, 4.0f);
                }
            );
            listenerIds.push_back({Managers::GameEventType::BossSpawned, bossSpawnedId});
        }

        void cleanup()
        {
            // Unsubscribe all listeners
            for (const auto& pair : listenerIds)
            {
                Managers::EventManager::getInstance().unsubscribe(pair.first, pair.second);
            }
            listenerIds.clear();
        }

        void showNotification(const std::string& text, sf::Color color, float duration)
        {
            currentNotification.text = text;
            currentNotification.color = color;
            currentNotification.lifetime = duration;
            currentNotification.maxLifetime = duration;
            currentNotification.alpha = 255.0f;
            currentNotification.active = true;
        }

        void update(sf::Time dt)
        {
            if (!currentNotification.active)
                return;

            currentNotification.lifetime -= dt.asSeconds();

            // Start fading when less than 0.5s remaining
            if (currentNotification.lifetime < 0.5f && currentNotification.lifetime > 0)
            {
                currentNotification.alpha = (currentNotification.lifetime / 0.5f) * 255.0f;
            }

            // Deactivate when lifetime expires
            if (currentNotification.lifetime <= 0)
            {
                currentNotification.active = false;
            }
        }

        void render(sf::RenderWindow& window)
        {
            if (!currentNotification.active)
                return;

            // Get window size
            sf::Vector2u windowSize = window.getSize();

            // Position at center horizontally, 80px above vertical center
            const float VERTICAL_OFFSET = 80.f;
            sf::Vector2f notificationPos(windowSize.x / 2.0f, (windowSize.y / 2.0f) - VERTICAL_OFFSET);

            // Create text
            sf::Text text(font);
            text.setString(currentNotification.text);
            text.setCharacterSize(32);

            // Apply alpha
            sf::Color colorWithAlpha = currentNotification.color;
            colorWithAlpha.a = static_cast<uint8_t>(currentNotification.alpha);
            text.setFillColor(colorWithAlpha);

            // Center text horizontally
            sf::FloatRect textBounds = text.getLocalBounds();
            text.setOrigin(sf::Vector2f(
                textBounds.position.x + textBounds.size.x / 2.0f,
                textBounds.position.y + textBounds.size.y / 2.0f
            ));
            text.setPosition(notificationPos);

            // Draw background box
            sf::RectangleShape background;
            background.setSize(sf::Vector2f(textBounds.size.x + 40.f, textBounds.size.y + 30.f));
            background.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(currentNotification.alpha * 0.7f)));
            background.setOutlineColor(sf::Color(255, 255, 255, static_cast<uint8_t>(currentNotification.alpha * 0.5f)));
            background.setOutlineThickness(2.f);
            background.setOrigin(sf::Vector2f(background.getSize().x / 2.0f, background.getSize().y / 2.0f));
            background.setPosition(notificationPos);

            window.draw(background);
            window.draw(text);
        }

    private:
        struct Notification
        {
            std::string text;
            sf::Color color;
            float lifetime;
            float maxLifetime;
            float alpha;
            bool active = false;
        };

        sf::Color getColorForBuff(const std::string& buffName)
        {
            if (buffName.find("Damage") != std::string::npos)
                return sf::Color(255, 100, 100);  // Red
            else if (buffName.find("Speed") != std::string::npos)
                return sf::Color(100, 255, 255);  // Cyan
            else if (buffName.find("Invulnerability") != std::string::npos)
                return sf::Color(255, 255, 100);  // Yellow
            else if (buffName.find("XP") != std::string::npos)
                return sf::Color(255, 100, 255);  // Magenta
            else if (buffName.find("Health") != std::string::npos || buffName.find("Regen") != std::string::npos)
                return sf::Color(100, 255, 100);  // Green
            else if (buffName.find("Fire") != std::string::npos)
                return sf::Color(255, 165, 0);    // Orange
            else
                return sf::Color::White;
        }

        Notification currentNotification;
        std::vector<std::pair<Managers::GameEventType, int>> listenerIds;
        const sf::Font& font;
    };
}
