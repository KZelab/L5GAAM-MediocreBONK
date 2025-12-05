#pragma once
#include <SFML/Graphics.hpp>
#include "../Core/ResourceManager.h"
#include "../ECS/Entity.h"
#include "../ECS/Components/Health.h"
#include "../ECS/Components/Experience.h"
#include "BuffDisplay.h"
#include <sstream>
#include <iomanip>
#include <memory>

namespace MediocreBONK::UI
{
    class HUD
    {
    public:
        HUD(ECS::Entity* player)
            : player(player)
            , gameTime(0.f)
            , killCount(0)
            , fps(0.f)
            , fpsUpdateInterval(0.25f) // Update FPS display every 0.25 seconds
            , frameCount(0)
            , font(Core::ResourceManager::getInstance().getFont("assets/fonts/arial.ttf"))
            , buffDisplay(nullptr)
        {
            // Font loaded via ResourceManager

            // Initialize buff display (position will be set dynamically in render)
            buffDisplay = std::make_unique<BuffDisplay>(sf::Vector2f(0.f, 100.f));

            // Start FPS timer using real wall clock time
            fpsClock.restart();
        }

        void update(sf::Time dt)
        {
            gameTime += dt.asSeconds();

            // Update buff display
            if (buffDisplay)
            {
                buffDisplay->update(player, dt);
            }
        }

        void incrementKillCount()
        {
            killCount++;
        }

        float getGameTime() const { return gameTime; }
        int getKillCount() const { return killCount; }

        void render(sf::RenderWindow& window)
        {
            // Update FPS counter - count actual rendered frames
            frameCount++;
            float elapsedTime = fpsClock.getElapsedTime().asSeconds();

            if (elapsedTime >= fpsUpdateInterval)
            {
                fps = frameCount / elapsedTime;
                frameCount = 0;
                fpsClock.restart();
            }

            auto* health = player->getComponent<ECS::Components::Health>();
            auto* experience = player->getComponent<ECS::Components::Experience>();

            if (!health || !experience)
                return;

            sf::Vector2u windowSize = window.getSize();

            // Draw health bar (top-left)
            drawHealthBar(window, health, sf::Vector2f(20.f, 20.f), sf::Vector2f(200.f, 20.f));

            // Draw XP bar (below health bar)
            drawXPBar(window, experience, sf::Vector2f(20.f, 50.f), sf::Vector2f(200.f, 15.f));

            // Draw level (next to XP bar)
            drawLevel(window, experience, sf::Vector2f(230.f, 48.f));

            // Draw timer (top-center)
            drawTimer(window, sf::Vector2f(windowSize.x / 2.f, 20.f));

            // Draw kill count (top-right)
            drawKillCount(window, sf::Vector2f(windowSize.x - 150.f, 20.f));

            // Draw FPS counter (top-right, below kill count)
            drawFPS(window, sf::Vector2f(windowSize.x - 150.f, 60.f));

            // Draw buff display (top-right, below FPS)
            if (buffDisplay)
            {
                buffDisplay->render(window, sf::Vector2f(windowSize.x - 140.f, 100.f));
            }
        }

    private:
        void drawHealthBar(sf::RenderWindow& window, ECS::Components::Health* health,
                          const sf::Vector2f& position, const sf::Vector2f& size)
        {
            // Background (dark red)
            sf::RectangleShape background(size);
            background.setPosition(position);
            background.setFillColor(sf::Color(100, 0, 0));
            background.setOutlineThickness(2.f);
            background.setOutlineColor(sf::Color::White);
            window.draw(background);

            // Foreground (bright red, scaled by health percentage)
            float healthPercent = health->getHealthPercentage();
            sf::RectangleShape foreground(sf::Vector2f(size.x * healthPercent, size.y));
            foreground.setPosition(position);
            foreground.setFillColor(sf::Color::Red);
            window.draw(foreground);

            // Health text
            sf::Text healthText(font);
            healthText.setCharacterSize(14);
            healthText.setFillColor(sf::Color::White);
            healthText.setString(std::to_string(static_cast<int>(health->currentHealth)) + " / " +
                                 std::to_string(static_cast<int>(health->maxHealth)));
            healthText.setPosition({position.x + 5.f, position.y + 2.f});
            window.draw(healthText);
        }

        void drawXPBar(sf::RenderWindow& window, ECS::Components::Experience* experience,
                      const sf::Vector2f& position, const sf::Vector2f& size)
        {
            // Background (dark yellow/gold)
            sf::RectangleShape background(size);
            background.setPosition(position);
            background.setFillColor(sf::Color(100, 100, 0));
            background.setOutlineThickness(2.f);
            background.setOutlineColor(sf::Color::White);
            window.draw(background);

            // Foreground (bright yellow/gold, scaled by XP percentage)
            float xpPercent = experience->getXPPercentage();
            sf::RectangleShape foreground(sf::Vector2f(size.x * xpPercent, size.y));
            foreground.setPosition(position);
            foreground.setFillColor(sf::Color::Yellow);
            window.draw(foreground);
        }

        void drawLevel(sf::RenderWindow& window, ECS::Components::Experience* experience,
                      const sf::Vector2f& position)
        {
            int level = experience->getCurrentLevel();

            // Level text
            sf::Text levelText(font);
            levelText.setCharacterSize(16);
            levelText.setFillColor(sf::Color::Cyan);
            levelText.setString("Lvl " + std::to_string(level));
            levelText.setPosition(position);
            levelText.setOutlineThickness(1.f);
            levelText.setOutlineColor(sf::Color::Black);
            window.draw(levelText);
        }

        void drawTimer(sf::RenderWindow& window, const sf::Vector2f& position)
        {
            // Format time as MM:SS
            int minutes = static_cast<int>(gameTime) / 60;
            int seconds = static_cast<int>(gameTime) % 60;

            // Draw timer box
            sf::RectangleShape timerBox(sf::Vector2f(100.f, 30.f));
            timerBox.setPosition(position);
            timerBox.setOrigin({50.f, 0.f}); // Center horizontally
            timerBox.setFillColor(sf::Color(50, 50, 50, 200));
            timerBox.setOutlineThickness(2.f);
            timerBox.setOutlineColor(sf::Color::White);
            window.draw(timerBox);

            // Timer text
            std::ostringstream timeStream;
            timeStream << std::setfill('0') << std::setw(2) << minutes << ":"
                      << std::setfill('0') << std::setw(2) << seconds;

            sf::Text timerText(font);
            timerText.setCharacterSize(18);
            timerText.setFillColor(sf::Color::White);
            timerText.setString(timeStream.str());
            sf::FloatRect textBounds = timerText.getLocalBounds();
            timerText.setOrigin({textBounds.size.x / 2.f, 0.f});
            timerText.setPosition({position.x, position.y + 5.f});
            window.draw(timerText);
        }

        void drawKillCount(sf::RenderWindow& window, const sf::Vector2f& position)
        {
            // Draw kill count box
            sf::RectangleShape killBox(sf::Vector2f(130.f, 30.f));
            killBox.setPosition(position);
            killBox.setFillColor(sf::Color(50, 50, 50, 200));
            killBox.setOutlineThickness(2.f);
            killBox.setOutlineColor(sf::Color::White);
            window.draw(killBox);

            // Kill count text
            sf::Text killText(font);
            killText.setCharacterSize(18);
            killText.setFillColor(sf::Color(255, 200, 0)); // Orange/yellow
            killText.setString("Kills: " + std::to_string(killCount));
            killText.setPosition({position.x + 10.f, position.y + 5.f});
            window.draw(killText);
        }

        void drawFPS(sf::RenderWindow& window, const sf::Vector2f& position)
        {
            // Draw FPS box
            sf::RectangleShape fpsBox(sf::Vector2f(130.f, 30.f));
            fpsBox.setPosition(position);
            fpsBox.setFillColor(sf::Color(50, 50, 50, 200));
            fpsBox.setOutlineThickness(2.f);
            fpsBox.setOutlineColor(sf::Color::White);
            window.draw(fpsBox);

            // Choose color based on FPS (green = good, yellow = ok, red = bad)
            sf::Color fpsColor;
            if (fps >= 55.f)
                fpsColor = sf::Color::Green;
            else if (fps >= 30.f)
                fpsColor = sf::Color::Yellow;
            else
                fpsColor = sf::Color::Red;

            // FPS text
            sf::Text fpsText(font);
            fpsText.setCharacterSize(18);
            fpsText.setFillColor(fpsColor);
            fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
            fpsText.setPosition({position.x + 10.f, position.y + 5.f});
            window.draw(fpsText);
        }

        ECS::Entity* player;
        float gameTime;
        int killCount;
        const sf::Font& font;

        // FPS tracking
        float fps;
        float fpsUpdateInterval;
        int frameCount;
        sf::Clock fpsClock; // Real wall clock time for accurate FPS

        // Buff display
        std::unique_ptr<BuffDisplay> buffDisplay;
    };
}
