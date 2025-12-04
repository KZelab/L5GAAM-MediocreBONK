#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

namespace MediocreBONK::Core
{
    class StateMachine; // Forward declaration
}

namespace MediocreBONK::States
{
    class State
    {
    public:
        virtual ~State() = default;

        // Called when state is pushed onto the stack
        virtual void enter() = 0;

        // Called when state is popped from the stack
        virtual void exit() = 0;

        // Update game logic
        virtual void update(sf::Time dt) = 0;

        // Render to window
        virtual void render(sf::RenderWindow& window) = 0;

        // Handle input events
        virtual void handleInput(const sf::Event& event) = 0;

        void setStateMachine(Core::StateMachine* sm) { stateMachine = sm; }

    protected:
        Core::StateMachine* stateMachine = nullptr;

        friend class Core::StateMachine;
    };
}
