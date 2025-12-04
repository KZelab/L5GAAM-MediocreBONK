#pragma once
#include "../States/State.h"
#include <memory>
#include <stack>

namespace MediocreBONK::Core
{
    /*
     * DESIGN PATTERN: STATE MACHINE PATTERN
     *
     * Purpose:
     * - Manage game state transitions (Menu -> Game -> Death -> Menu)
     * - Each state has its own update/render/input logic
     * - Clean state transitions with enter/exit hooks
     *
     * Implementation:
     * - Stack-based state machine (states can be pushed/popped)
     * - Each state implements the State interface
     * - States are stored as unique_ptr (automatic memory management)
     *
     * Benefits:
     * - Separation of concerns: each state is self-contained
     * - Easy to add new states without modifying existing code
     * - Stack structure allows pause/resume (e.g., pause menu over game)
     * - Clear lifecycle: enter() on push, exit() on pop
     *
     * State Machine Transitions:
     * - pushState: Add new state on top (pauses previous state)
     * - popState: Remove current state (returns to previous)
     * - changeState: Replace current state (pop + push)
     *
     * Example Flow:
     * 1. Start: pushState(MenuState)
     * 2. Press Start: changeState(GameState)  [Menu.exit() -> Game.enter()]
     * 3. Player dies: changeState(DeathState) [Game.exit() -> Death.enter()]
     * 4. Press Restart: changeState(GameState) [Death.exit() -> Game.enter()]
     */
    class StateMachine
    {
    public:
        StateMachine() = default;
        ~StateMachine() = default;

        // STATE TRANSITION: Push a new state on top of the stack
        // Used for temporary states (e.g., pause menu over game)
        // Previous state is paused (exit() called but stays in memory)
        void pushState(std::unique_ptr<States::State> state)
        {
            if (!states.empty())
            {
                states.top()->exit(); // Pause current state
            }

            // Give state access to the state machine (for transitions)
            state->stateMachine = this;
            state->enter(); // Initialize new state
            states.push(std::move(state)); // Transfer ownership to stack
        }

        // STATE TRANSITION: Remove current state and return to previous
        // Used for closing temporary states (e.g., closing pause menu)
        void popState()
        {
            if (!states.empty())
            {
                states.top()->exit(); // Clean up current state
                states.pop(); // Remove from stack (destroys unique_ptr)

                if (!states.empty())
                {
                    states.top()->enter(); // Resume previous state
                }
            }
        }

        // STATE TRANSITION: Replace current state with a new one
        // Used for major transitions (Menu -> Game, Game -> Death)
        // Combines pop + push in one operation
        void changeState(std::unique_ptr<States::State> state)
        {
            if (!states.empty())
            {
                states.top()->exit(); // Clean up old state
                states.pop(); // Remove old state (memory freed)
            }

            state->stateMachine = this;
            state->enter(); // Initialize new state
            states.push(std::move(state)); // New state becomes active
        }

        // Delegate update to the active (top) state
        void update(sf::Time dt)
        {
            if (!states.empty())
            {
                states.top()->update(dt); // Only active state updates
            }
        }

        // Delegate rendering to the active (top) state
        void render(sf::RenderWindow& window)
        {
            if (!states.empty())
            {
                states.top()->render(window); // Only active state renders
            }
        }

        // Delegate input handling to the active (top) state
        void handleInput(const sf::Event& event)
        {
            if (!states.empty())
            {
                states.top()->handleInput(event); // Only active state handles input
            }
        }

        // Check if state machine has any states
        // Used by Game loop to determine when to exit
        bool isEmpty() const
        {
            return states.empty();
        }

    private:
        // Stack of states: top = active state
        // std::unique_ptr provides automatic memory management
        // Stack structure enables pause/resume behavior
        std::stack<std::unique_ptr<States::State>> states;
    };
}
