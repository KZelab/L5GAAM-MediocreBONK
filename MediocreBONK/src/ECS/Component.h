#pragma once
#include <SFML/Graphics.hpp>

namespace MediocreBONK::ECS
{
    class Entity; // Forward declaration

    /*
     * DESIGN PATTERN: ENTITY-COMPONENT-SYSTEM (ECS) - Component Base Class
     *
     * What is ECS?
     * - Compositional game architecture (alternative to inheritance hierarchies)
     * - Entities are containers (like game objects)
     * - Components are pure data + behavior (Transform, Health, Sprite, etc.)
     * - Systems process entities with specific component combinations
     *
     * Component (this class):
     * - Base class for all component types
     * - Contains common interface (update, render, lifecycle hooks)
     * - Components know their owner entity (can access sibling components)
     *
     * Example Components in this project:
     * - Transform: position, rotation, scale
     * - Health: currentHealth, maxHealth
     * - Sprite: visual representation
     * - Collider: collision detection
     * - Weapon: attack behavior
     *
     * ECS Benefits:
     * - Flexible composition (mix and match components)
     * - No deep inheritance hierarchies (avoid diamond problem)
     * - Data-oriented design (cache-friendly for large entity counts)
     * - Easy to add new component types without modifying existing code
     *
     * ECS vs Traditional OOP:
     * - Traditional: Player extends Character extends GameObject
     * - ECS: Player = Entity + [Transform, Health, Sprite, Input, Weapon]
     *
     * DESIGN PATTERN: TEMPLATE METHOD PATTERN
     * - Base class defines lifecycle hooks (onAttach, onDetach, update, render)
     * - Derived components override only what they need
     * - Default implementations do nothing (empty methods)
     */
    class Component
    {
    public:
        virtual ~Component() = default;

        // LIFECYCLE: Update component logic each frame
        // Override in derived classes to implement component-specific behavior
        virtual void update(sf::Time dt) {}

        // LIFECYCLE: Render component (if visual)
        // Not all components render (e.g., Health, Collider)
        virtual void render(sf::RenderWindow& window) {}

        // LIFECYCLE: Called when component is added to entity
        // Use for initialization that requires owner entity to exist
        virtual void onAttach() {}

        // LIFECYCLE: Called when component is removed from entity
        // Use for cleanup (unregister from systems, release resources)
        virtual void onDetach() {}

        // Back-reference to owning entity
        // Allows component to access sibling components
        // Example: Sprite can read Transform to know where to draw
        Entity* owner = nullptr;

        // Enable/disable component without removing it
        // Inactive components are skipped in update/render
        bool active = true;
    };
}
