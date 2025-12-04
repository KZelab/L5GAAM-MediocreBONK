#pragma once
#include "State.h"
#include "../Core/ResourceManager.h"
#include "../Utils/Logger.h"

// Forward declaration to avoid circular dependency
namespace MediocreBONK::States
{
    class GameState;
}

namespace MediocreBONK::States
{
    class MenuState : public State
    {
    public:
        MenuState()
            : font(Core::ResourceManager::getInstance().getFont("assets/fonts/arial.ttf"))
        {}

        void enter() override
        {
            Utils::Logger::info("Entered Menu State");
        }

        void exit() override
        {
            Utils::Logger::info("Exited Menu State");
        }

        void update(sf::Time dt) override
        {
            // Menu update logic (future: button animations, etc.)
        }

        void render(sf::RenderWindow& window) override
        {
            // Simple visual for now - just a colored background
            sf::Vector2u winSize = window.getSize();
            sf::RectangleShape background(sf::Vector2f(static_cast<float>(winSize.x), static_cast<float>(winSize.y)));
            background.setFillColor(sf::Color(20, 20, 40)); // Dark blue
            window.draw(background);

            // Title text
            sf::Text titleText(font);
            titleText.setCharacterSize(72);
            titleText.setFillColor(sf::Color::White);
            titleText.setString("MediocreBONK");
            titleText.setOutlineThickness(3.f);
            titleText.setOutlineColor(sf::Color(100, 100, 255));
            sf::FloatRect titleBounds = titleText.getLocalBounds();
            titleText.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
            titleText.setPosition({static_cast<float>(winSize.x) / 2.f, static_cast<float>(winSize.y) / 3.f});
            window.draw(titleText);

            // Start instruction text
            sf::Text startText(font);
            startText.setCharacterSize(28);
            startText.setFillColor(sf::Color::Yellow);
            startText.setString("Press SPACE to Start");
            sf::FloatRect startBounds = startText.getLocalBounds();
            startText.setOrigin({startBounds.size.x / 2.f, startBounds.size.y / 2.f});
            startText.setPosition({static_cast<float>(winSize.x) / 2.f, static_cast<float>(winSize.y) / 2.f});
            window.draw(startText);

            // Exit instruction text
            sf::Text exitText(font);
            exitText.setCharacterSize(20);
            exitText.setFillColor(sf::Color(200, 200, 200));
            exitText.setString("Press ESCAPE to Exit");
            sf::FloatRect exitBounds = exitText.getLocalBounds();
            exitText.setOrigin({exitBounds.size.x / 2.f, exitBounds.size.y / 2.f});
            exitText.setPosition({static_cast<float>(winSize.x) / 2.f, static_cast<float>(winSize.y) / 2.f + 50.f});
            window.draw(exitText);
        }

        void handleInput(const sf::Event& event) override;

    private:
        void startGame();
        const sf::Font& font;
    };
}

// Include GameState here to avoid circular dependency
#include "GameState.h"

namespace MediocreBONK::States
{
    inline void MenuState::handleInput(const sf::Event& event)
    {
        // Check for key press to start game
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Space)
            {
                Utils::Logger::info("Space pressed - transitioning to GameState");
                startGame();
            }

            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                Utils::Logger::info("Escape pressed - exiting");
                // Will cause game loop to exit
                stateMachine->popState();
            }
        }
    }

    inline void MenuState::startGame()
    {
        stateMachine->changeState(std::unique_ptr<State>(new GameState()));
    }
}
