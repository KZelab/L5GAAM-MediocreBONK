#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.h"
#include "ResourceManager.h"
#include "../Utils/Logger.h"
#include <memory>

namespace MediocreBONK::Core
{
    /*
     * DESIGN PATTERN: GAME LOOP WITH FIXED TIMESTEP
     *
     * Purpose:
     * - Decouple rendering from game logic updates
     * - Ensure consistent physics/gameplay regardless of frame rate
     * - Smooth rendering even if updates are slower than display refresh
     *
     * Fixed Timestep Benefits:
     * - Deterministic simulation (same input = same output)
     * - Network games stay in sync (all clients update at same rate)
     * - Physics calculations are stable (no "fast computer advantage")
     * - Easy to replay/record gameplay
     *
     * Implementation:
     * - Update runs at fixed 60 Hz (16.67ms per update)
     * - Render runs as fast as possible (vsync to 60 FPS)
     * - If frame takes too long, multiple updates run to catch up
     * - If frame is fast, render interpolates between updates (not implemented here)
     *
     * Alternative Approaches:
     * - Variable timestep: dt changes each frame (simpler but non-deterministic)
     * - Semi-fixed timestep: cap max dt to prevent "spiral of death"
     *
     * Reference: "Fix Your Timestep!" by Glenn Fiedler
     * https://gafferongames.com/post/fix_your_timestep/
     */
    class Game
    {
    public:
        Game()
            : window(sf::VideoMode({1920, 1080}), "MediocreBONK")
            , stateMachine(std::make_unique<StateMachine>())
        {
            window.setFramerateLimit(60); // Soft FPS cap (not guaranteed)
            Utils::Logger::info("Game initialized");
        }

        ~Game()
        {
            Utils::Logger::info("Game shutting down");
        }

        // GAME LOOP: Fixed timestep for deterministic simulation
        void run()
        {
            sf::Clock clock;
            sf::Time timeSinceLastUpdate = sf::Time::Zero;
            const sf::Time timePerFrame = sf::seconds(1.f / 60.f); // 60 updates per second

            // Main game loop: runs until window closed or no states remain
            while (window.isOpen() && !stateMachine->isEmpty())
            {
                // Measure time elapsed since last frame
                sf::Time deltaTime = clock.restart();
                timeSinceLastUpdate += deltaTime;

                // FIXED TIMESTEP: Update in fixed increments (16.67ms each)
                // Multiple updates may run if frame took too long
                // This ensures physics runs at consistent speed
                while (timeSinceLastUpdate > timePerFrame)
                {
                    timeSinceLastUpdate -= timePerFrame;
                    processInput();           // Handle user input
                    update(timePerFrame);     // Update game logic (always 16.67ms)
                }

                // Render current frame (runs as fast as possible)
                render();

                // Note: Could add interpolation here for smoother visuals
                // float interpolation = timeSinceLastUpdate / timePerFrame;
                // render(interpolation);
            }
        }

        StateMachine& getStateMachine()
        {
            return *stateMachine;
        }

        sf::RenderWindow& getWindow()
        {
            return window;
        }

    private:
        // Process all input events accumulated since last frame
        void processInput()
        {
            // SFML 3 event handling: poll all events in queue
            while (auto event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }

                // Delegate event to active state (State Machine pattern)
                stateMachine->handleInput(*event);
            }
        }

        // Update game logic with fixed timestep (always 16.67ms)
        void update(sf::Time dt)
        {
            // Delegate update to active state
            stateMachine->update(dt);
        }

        // Render current frame to screen
        void render()
        {
            window.clear(sf::Color::Black);  // Clear previous frame
            stateMachine->render(window);     // Draw current state
            window.display();                 // Swap buffers (show frame)
        }

        // Core game components
        sf::RenderWindow window;                        // SFML window (manages OpenGL context)
        std::unique_ptr<StateMachine> stateMachine;     // Manages game states
    };
}
