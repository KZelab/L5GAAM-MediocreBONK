#pragma once
#include "../ECS/Entity.h"
#include "../ECS/Components/Buff.h"
#include "../Core/ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace MediocreBONK::UI
{
    class BuffDisplay
    {
    public:
        BuffDisplay(const sf::Vector2f& position)
            : position(position)
            , font(Core::ResourceManager::getInstance().getFont("assets/fonts/arial.ttf"))
        {
        }

        void update(ECS::Entity* player, sf::Time dt)
        {
            displayedBuffs.clear();

            if (!player)
                return;

            auto* buff = player->getComponent<ECS::Components::Buff>();
            if (!buff)
                return;

            // Get active buffs and convert to display info
            const auto& activeBuffs = buff->getActiveBuffs();
            for (const auto& buffEffect : activeBuffs)
            {
                BuffDisplayInfo info;
                info.name = buffEffect.name;
                info.type = buffEffect.type;
                info.remainingTime = buffEffect.remainingTime;
                info.totalDuration = buffEffect.duration;
                info.color = getBuffColor(buffEffect.type);
                info.symbol = getBuffSymbol(buffEffect.type);

                displayedBuffs.push_back(info);
            }
        }

        void render(sf::RenderWindow& window, const sf::Vector2f& renderPosition)
        {
            if (displayedBuffs.empty())
                return;

            float yOffset = 0.f;
            const float BOX_WIDTH = 120.f;
            const float BOX_HEIGHT = 30.f;
            const float SPACING = 5.f;

            for (const auto& buff : displayedBuffs)
            {
                sf::Vector2f buffPos(renderPosition.x, renderPosition.y + yOffset);

                // Draw background box
                sf::RectangleShape background;
                background.setSize(sf::Vector2f(BOX_WIDTH, BOX_HEIGHT));
                background.setPosition(buffPos);
                background.setFillColor(sf::Color(0, 0, 0, 180));
                background.setOutlineColor(buff.color);
                background.setOutlineThickness(2.f);
                window.draw(background);

                // Draw buff symbol
                sf::Text symbolText(font);
                symbolText.setString(std::string(1, buff.symbol));
                symbolText.setCharacterSize(20);
                symbolText.setFillColor(buff.color);
                symbolText.setPosition(sf::Vector2f(buffPos.x + 5.f, buffPos.y + 5.f));
                window.draw(symbolText);

                // Draw timer text
                std::string timerStr;
                if (buff.remainingTime > 0.f)
                {
                    timerStr = formatTime(buff.remainingTime);
                }
                else
                {
                    timerStr = "∞"; // Permanent buff
                }

                sf::Text timerText(font);
                timerText.setString(timerStr);
                timerText.setCharacterSize(16);
                timerText.setFillColor(sf::Color::White);
                sf::FloatRect timerBounds = timerText.getLocalBounds();
                timerText.setPosition(sf::Vector2f(
                    buffPos.x + BOX_WIDTH - timerBounds.size.x - 5.f,
                    buffPos.y + 7.f
                ));
                window.draw(timerText);

                // Draw timer bar
                if (buff.remainingTime > 0.f && buff.totalDuration > 0.f)
                {
                    float timePercent = buff.remainingTime / buff.totalDuration;
                    sf::RectangleShape timerBar;
                    timerBar.setSize(sf::Vector2f((BOX_WIDTH - 4.f) * timePercent, 3.f));
                    timerBar.setPosition(sf::Vector2f(buffPos.x + 2.f, buffPos.y + BOX_HEIGHT - 5.f));
                    timerBar.setFillColor(buff.color);
                    window.draw(timerBar);
                }

                yOffset += BOX_HEIGHT + SPACING;
            }
        }

    private:
        struct BuffDisplayInfo
        {
            std::string name;
            ECS::Components::BuffType type;
            float remainingTime;
            float totalDuration;
            sf::Color color;
            char symbol;
        };

        char getBuffSymbol(ECS::Components::BuffType type) const
        {
            switch (type)
            {
            case ECS::Components::BuffType::DamageBoost:
                return 'D';
            case ECS::Components::BuffType::SpeedBoost:
                return 'S';
            case ECS::Components::BuffType::InvulnerabilityBoost:
                return 'I';
            case ECS::Components::BuffType::XPMultiplier:
                return 'X';
            case ECS::Components::BuffType::HealthRegen:
                return 'H';
            case ECS::Components::BuffType::FireRateBoost:
                return 'F';
            case ECS::Components::BuffType::MagnetRange:
                return 'M';
            default:
                return '?';
            }
        }

        sf::Color getBuffColor(ECS::Components::BuffType type) const
        {
            switch (type)
            {
            case ECS::Components::BuffType::DamageBoost:
                return sf::Color(255, 100, 100);  // Red
            case ECS::Components::BuffType::SpeedBoost:
                return sf::Color(100, 255, 255);  // Cyan
            case ECS::Components::BuffType::InvulnerabilityBoost:
                return sf::Color(255, 255, 100);  // Yellow
            case ECS::Components::BuffType::XPMultiplier:
                return sf::Color(255, 100, 255);  // Magenta
            case ECS::Components::BuffType::HealthRegen:
                return sf::Color(100, 255, 100);  // Green
            case ECS::Components::BuffType::FireRateBoost:
                return sf::Color(255, 165, 0);    // Orange
            case ECS::Components::BuffType::MagnetRange:
                return sf::Color(255, 215, 0);    // Gold
            default:
                return sf::Color::White;
            }
        }

        std::string formatTime(float seconds) const
        {
            if (seconds < 10.f)
            {
                // Show one decimal place for < 10 seconds
                int wholePart = static_cast<int>(seconds);
                int decimalPart = static_cast<int>((seconds - wholePart) * 10.f);
                return std::to_string(wholePart) + "." + std::to_string(decimalPart) + "s";
            }
            else
            {
                // Show whole seconds for >= 10 seconds
                return std::to_string(static_cast<int>(seconds)) + "s";
            }
        }

        sf::Vector2f position;
        std::vector<BuffDisplayInfo> displayedBuffs;
        const sf::Font& font;
    };
}
