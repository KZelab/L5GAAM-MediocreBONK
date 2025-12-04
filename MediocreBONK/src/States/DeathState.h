#pragma once
#include "State.h"
#include "../Utils/Logger.h"
#include <SFML/Graphics.hpp>

// Forward declarations to avoid circular dependencies
namespace MediocreBONK::States
{
    class GameState;
    class MenuState;
}

namespace MediocreBONK::States
{
    class DeathState : public State
    {
    public:
        DeathState(float survivalTime, int killCount, int level)
            : survivalTime(survivalTime)
            , killCount(killCount)
            , level(level)
        {}

        void enter() override
        {
            Utils::Logger::info("Entered Death State");
            Utils::Logger::info("Survival Time: " + std::to_string(survivalTime) + "s");
            Utils::Logger::info("Kills: " + std::to_string(killCount));
            Utils::Logger::info("Level: " + std::to_string(level));
        }

        void exit() override
        {
            Utils::Logger::info("Exited Death State");
        }

        void update(sf::Time dt) override
        {
            // Death screen is static, no updates needed
        }

        void render(sf::RenderWindow& window) override
        {
            sf::Vector2u windowSize = window.getSize();
            float centerX = windowSize.x / 2.f;
            float centerY = windowSize.y / 2.f;

            // Dark overlay background
            sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
            overlay.setFillColor(sf::Color(20, 0, 0, 220)); // Dark red tint
            window.draw(overlay);

            // "GAME OVER" title box
            sf::RectangleShape titleBox(sf::Vector2f(400.f, 80.f));
            titleBox.setPosition({centerX - 200.f, centerY - 200.f});
            titleBox.setFillColor(sf::Color(100, 0, 0));
            titleBox.setOutlineThickness(4.f);
            titleBox.setOutlineColor(sf::Color::Red);
            window.draw(titleBox);

            // Stats panel
            sf::RectangleShape statsBox(sf::Vector2f(500.f, 250.f));
            statsBox.setPosition({centerX - 250.f, centerY - 80.f});
            statsBox.setFillColor(sf::Color(40, 40, 40, 230));
            statsBox.setOutlineThickness(3.f);
            statsBox.setOutlineColor(sf::Color::White);
            window.draw(statsBox);

            // Survival time indicator
            drawStatBar(window, "TIME", survivalTime, 600.f,
                       sf::Vector2f(centerX - 200.f, centerY - 40.f),
                       sf::Color(100, 150, 255));

            // Kill count indicator
            drawStatBar(window, "KILLS", static_cast<float>(killCount), 500.f,
                       sf::Vector2f(centerX - 200.f, centerY + 20.f),
                       sf::Color(255, 100, 100));

            // Level indicator
            drawStatBar(window, "LEVEL", static_cast<float>(level), 50.f,
                       sf::Vector2f(centerX - 200.f, centerY + 80.f),
                       sf::Color(255, 255, 100));

            // Instructions box (bottom)
            sf::RectangleShape instructionsBox(sf::Vector2f(600.f, 60.f));
            instructionsBox.setPosition({centerX - 300.f, centerY + 180.f});
            instructionsBox.setFillColor(sf::Color(30, 30, 30, 200));
            instructionsBox.setOutlineThickness(2.f);
            instructionsBox.setOutlineColor(sf::Color(150, 150, 150));
            window.draw(instructionsBox);

            // Visual indicators for controls (shapes since no font)
            // Space = Restart indicator
            sf::RectangleShape spaceKey(sf::Vector2f(80.f, 30.f));
            spaceKey.setPosition({centerX - 120.f, centerY + 195.f});
            spaceKey.setFillColor(sf::Color(100, 200, 100));
            spaceKey.setOutlineThickness(2.f);
            spaceKey.setOutlineColor(sf::Color::White);
            window.draw(spaceKey);

            // Arrow pointing to "Restart"
            sf::CircleShape restartIndicator(8.f, 3);
            restartIndicator.setPosition({centerX - 25.f, centerY + 202.f});
            restartIndicator.setRotation(sf::degrees(90.f));
            restartIndicator.setFillColor(sf::Color(100, 200, 100));
            window.draw(restartIndicator);

            // Escape = Menu indicator
            sf::RectangleShape escKey(sf::Vector2f(80.f, 30.f));
            escKey.setPosition({centerX + 40.f, centerY + 195.f});
            escKey.setFillColor(sf::Color(200, 100, 100));
            escKey.setOutlineThickness(2.f);
            escKey.setOutlineColor(sf::Color::White);
            window.draw(escKey);

            // Arrow pointing to "Menu"
            sf::CircleShape menuIndicator(8.f, 3);
            menuIndicator.setPosition({centerX + 135.f, centerY + 202.f});
            menuIndicator.setRotation(sf::degrees(90.f));
            menuIndicator.setFillColor(sf::Color(200, 100, 100));
            window.draw(menuIndicator);

            // TODO: When font is available, replace visual indicators with text:
            // - "GAME OVER" title
            // - "Survival Time: MM:SS"
            // - "Kills: X"
            // - "Level: X"
            // - "Press SPACE to Restart"
            // - "Press ESC for Menu"
        }

        void handleInput(const sf::Event& event) override
        {
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    // Restart game - create new GameState
                    Utils::Logger::info("Restarting game...");
                    restartGame();
                }
                else if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    // Return to menu
                    Utils::Logger::info("Returning to menu...");
                    returnToMenu();
                }
            }
        }

    private:
        void restartGame();
        void returnToMenu();

        void drawStatBar(sf::RenderWindow& window, const std::string& label,
                        float value, float maxValue, const sf::Vector2f& position,
                        const sf::Color& color)
        {
            // Label indicator (small square with unique color)
            sf::RectangleShape labelBox(sf::Vector2f(80.f, 30.f));
            labelBox.setPosition(position);
            labelBox.setFillColor(color);
            labelBox.setOutlineThickness(2.f);
            labelBox.setOutlineColor(sf::Color::White);
            window.draw(labelBox);

            // Value bar background
            sf::RectangleShape background(sf::Vector2f(280.f, 30.f));
            background.setPosition({position.x + 90.f, position.y});
            background.setFillColor(sf::Color(50, 50, 50));
            background.setOutlineThickness(2.f);
            background.setOutlineColor(sf::Color::White);
            window.draw(background);

            // Value bar foreground (scaled)
            float percentage = std::min(value / maxValue, 1.f);
            sf::RectangleShape foreground(sf::Vector2f(280.f * percentage, 30.f));
            foreground.setPosition({position.x + 90.f, position.y});
            foreground.setFillColor(color);
            window.draw(foreground);

            // Value indicator circles (visual representation of magnitude)
            int circles = static_cast<int>(value / (maxValue / 10.f));
            circles = std::min(circles, 10);
            for (int i = 0; i < circles; ++i)
            {
                sf::CircleShape dot(4.f);
                dot.setPosition({position.x + 380.f + i * 12.f, position.y + 11.f});
                dot.setFillColor(color);
                window.draw(dot);
            }
        }

        float survivalTime;
        int killCount;
        int level;
    };
}

// Include state headers here to avoid circular dependencies
#include "GameState.h"
#include "MenuState.h"

namespace MediocreBONK::States
{
    inline void DeathState::restartGame()
    {
        stateMachine->changeState(std::unique_ptr<State>(new GameState()));
    }

    inline void DeathState::returnToMenu()
    {
        stateMachine->changeState(std::unique_ptr<State>(new MenuState()));
    }
}
