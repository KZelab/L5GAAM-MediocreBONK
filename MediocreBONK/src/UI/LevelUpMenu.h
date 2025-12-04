#pragma once
#include <SFML/Graphics.hpp>
#include "../Core/ResourceManager.h"
#include "../Managers/UpgradeManager.h"
#include <vector>

namespace MediocreBONK::UI
{
    class LevelUpMenu
    {
    public:
        LevelUpMenu()
            : isVisible(false)
            , selectedUpgrade(nullptr)
            , font(Core::ResourceManager::getInstance().getFont("assets/fonts/arial.ttf"))
        {
        }

        void show(ECS::Entity* player)
        {
            isVisible = true;
            this->player = player;

            // Get 3 random upgrades
            upgradeChoices = Managers::UpgradeManager::getInstance().getRandomUpgrades(3);

            // If no upgrades available (all maxed), auto-close menu
            if (upgradeChoices.empty())
            {
                hide();
                return;
            }

            selectedUpgrade = nullptr;
        }

        void hide()
        {
            isVisible = false;
            upgradeChoices.clear();
            selectedUpgrade = nullptr;
        }

        bool getIsVisible() const { return isVisible; }

        void handleInput(const sf::Event& event)
        {
            if (!isVisible)
                return;

            // Handle keyboard selection (1, 2, 3 keys)
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
            {
                int choice = -1;

                if (keyPressed->code == sf::Keyboard::Key::Num1)
                    choice = 0;
                else if (keyPressed->code == sf::Keyboard::Key::Num2)
                    choice = 1;
                else if (keyPressed->code == sf::Keyboard::Key::Num3)
                    choice = 2;

                if (choice >= 0 && choice < static_cast<int>(upgradeChoices.size()))
                {
                    selectUpgrade(upgradeChoices[choice]);
                }
            }
        }

        void render(sf::RenderWindow& window)
        {
            if (!isVisible)
                return;

            sf::Vector2u windowSize = window.getSize();

            // Draw semi-transparent overlay
            sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            window.draw(overlay);

            // Draw title box
            sf::RectangleShape titleBox(sf::Vector2f(400.f, 60.f));
            titleBox.setPosition({(windowSize.x - 400.f) / 2.f, 100.f});
            titleBox.setFillColor(sf::Color(50, 50, 100));
            titleBox.setOutlineThickness(3.f);
            titleBox.setOutlineColor(sf::Color::Yellow);
            window.draw(titleBox);

            // Draw title text
            sf::Text titleText(font);
            titleText.setCharacterSize(24);
            titleText.setFillColor(sf::Color::Yellow);
            titleText.setString("LEVEL UP!");
            sf::FloatRect titleBounds = titleText.getLocalBounds();
            titleText.setOrigin({titleBounds.size.x / 2.f, 0.f});
            titleText.setPosition({windowSize.x / 2.f, 115.f});
            window.draw(titleText);

            // Draw upgrade choice boxes
            float startY = 200.f;
            float spacing = 120.f;

            for (size_t i = 0; i < upgradeChoices.size(); ++i)
            {
                drawUpgradeBox(window, upgradeChoices[i], i + 1,
                    sf::Vector2f((windowSize.x - 500.f) / 2.f, startY + i * spacing),
                    sf::Vector2f(500.f, 100.f));
            }

            // Draw instruction text area
            sf::RectangleShape instructionBox(sf::Vector2f(400.f, 40.f));
            instructionBox.setPosition({(windowSize.x - 400.f) / 2.f, float(windowSize.y) - 100.f});
            instructionBox.setFillColor(sf::Color(40, 40, 40, 150));
            window.draw(instructionBox);

            // Draw instruction text
            sf::Text instructionText(font);
            instructionText.setCharacterSize(16);
            instructionText.setFillColor(sf::Color::White);
            instructionText.setString("Press 1, 2, or 3 to choose");
            sf::FloatRect instrBounds = instructionText.getLocalBounds();
            instructionText.setOrigin({instrBounds.size.x / 2.f, 0.f});
            instructionText.setPosition({windowSize.x / 2.f, float(windowSize.y) - 90.f});
            window.draw(instructionText);
        }

    private:
        void drawUpgradeBox(sf::RenderWindow& window, Managers::Upgrade* upgrade, int number,
                           const sf::Vector2f& position, const sf::Vector2f& size)
        {
            // Main box
            sf::RectangleShape box(size);
            box.setPosition(position);
            box.setFillColor(sf::Color(60, 60, 80));
            box.setOutlineThickness(3.f);
            box.setOutlineColor(sf::Color::White);
            window.draw(box);

            // Number indicator (left side)
            sf::CircleShape numberCircle(20.f);
            numberCircle.setPosition({position.x + 15.f, position.y + size.y / 2.f - 20.f});
            numberCircle.setFillColor(sf::Color::Yellow);
            numberCircle.setOutlineThickness(2.f);
            numberCircle.setOutlineColor(sf::Color::White);
            window.draw(numberCircle);

            // Color indicator based on upgrade type
            sf::Color typeColor;
            switch (upgrade->type)
            {
            case Managers::UpgradeType::DamageIncrease:
                typeColor = sf::Color::Red;
                break;
            case Managers::UpgradeType::FireRateIncrease:
                typeColor = sf::Color(255, 165, 0); // Orange
                break;
            case Managers::UpgradeType::ProjectileCountIncrease:
                typeColor = sf::Color::Yellow;
                break;
            case Managers::UpgradeType::HealthIncrease:
                typeColor = sf::Color::Green;
                break;
            case Managers::UpgradeType::SpeedIncrease:
                typeColor = sf::Color::Cyan;
                break;
            case Managers::UpgradeType::PiercingIncrease:
                typeColor = sf::Color::Magenta;
                break;
            default:
                typeColor = sf::Color::White;
            }

            // Type indicator stripe (right side)
            sf::RectangleShape typeStripe(sf::Vector2f(10.f, size.y));
            typeStripe.setPosition({position.x + size.x - 10.f, position.y});
            typeStripe.setFillColor(typeColor);
            window.draw(typeStripe);

            // Level indicator bars (bottom)
            float barWidth = (size.x - 80.f) / upgrade->maxLevel;
            for (int i = 0; i < upgrade->maxLevel; ++i)
            {
                sf::RectangleShape levelBar(sf::Vector2f(barWidth - 2.f, 5.f));
                levelBar.setPosition({position.x + 60.f + i * barWidth, position.y + size.y - 10.f});

                if (i < upgrade->currentLevel)
                    levelBar.setFillColor(sf::Color::Yellow);
                else
                    levelBar.setFillColor(sf::Color(100, 100, 100));

                window.draw(levelBar);
            }

            // Draw number text
            sf::Text numberText(font);
            numberText.setCharacterSize(20);
            numberText.setFillColor(sf::Color::Black);
            numberText.setString(std::to_string(number));
            sf::FloatRect numBounds = numberText.getLocalBounds();
            numberText.setOrigin({numBounds.size.x / 2.f, numBounds.size.y / 2.f});
            numberText.setPosition({position.x + 35.f, position.y + size.y / 2.f - 5.f});
            window.draw(numberText);

            // Draw upgrade name
            sf::Text nameText(font);
            nameText.setCharacterSize(20);
            nameText.setFillColor(sf::Color::White);
            nameText.setString(upgrade->name);
            nameText.setPosition({position.x + 70.f, position.y + 15.f});
            window.draw(nameText);

            // Draw upgrade description
            sf::Text descText(font);
            descText.setCharacterSize(16);
            descText.setFillColor(sf::Color(200, 200, 200));
            descText.setString(upgrade->description);
            descText.setPosition({position.x + 70.f, position.y + 45.f});
            window.draw(descText);

            // Draw level text
            sf::Text levelText(font);
            levelText.setCharacterSize(14);
            levelText.setFillColor(sf::Color::Yellow);
            levelText.setString(std::to_string(upgrade->currentLevel) + "/" + std::to_string(upgrade->maxLevel));
            levelText.setPosition({position.x + 70.f, position.y + 70.f});
            window.draw(levelText);
        }

        void selectUpgrade(Managers::Upgrade* upgrade)
        {
            selectedUpgrade = upgrade;

            // Apply the upgrade
            Managers::UpgradeManager::getInstance().applyUpgrade(upgrade, player);

            // Hide menu
            hide();
        }

        bool isVisible;
        ECS::Entity* player;
        std::vector<Managers::Upgrade*> upgradeChoices;
        Managers::Upgrade* selectedUpgrade;
        const sf::Font& font;
    };
}
