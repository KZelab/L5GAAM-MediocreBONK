#pragma once
#include "Component.h"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <SFML/Graphics.hpp>

namespace MediocreBONK::ECS
{
    /*
     * DESIGN PATTERN: ENTITY-COMPONENT-SYSTEM (ECS) - Entity Container
     *
     * Entity (this class):
     * - Lightweight container for components
     * - Just an ID + component map (no game logic)
     * - Acts like a bag of components
     *
     * Key Operations:
     * - addComponent<T>(): Create and attach component of type T
     * - getComponent<T>(): Retrieve component of type T (returns nullptr if missing)
     * - hasComponent<T>(): Check if entity has component of type T
     * - removeComponent<T>(): Detach and destroy component
     *
     * Example Usage:
     *   auto* entity = entityManager->createEntity();
     *   entity->addComponent<Transform>(100.f, 200.f);
     *   entity->addComponent<Health>(100.f);
     *   entity->addComponent<Sprite>("player.png");
     *
     *   // Later, in a system:
     *   auto* transform = entity->getComponent<Transform>();
     *   transform->position.x += 10.f;
     *
     * Implementation Details:
     * - Components stored in hash map (std::type_index as key)
     * - O(1) lookup/add/remove operations
     * - std::unique_ptr ensures automatic memory management
     * - Static assertions enforce Component inheritance
     *
     * Tags and Layers:
     * - tag: String identifier ("Player", "Enemy", "Projectile")
     * - layer: Integer for rendering/collision layers
     * - Used for querying entities (e.g., "find all enemies")
     */
    class Entity
    {
    public:
        Entity(uint64_t id) : id(id), active(true) {}
        ~Entity() = default;

        // ECS: Add a component to this entity
        // Template parameters: T = component type, Args = constructor arguments
        // Returns: Pointer to newly created component
        template<typename T, typename... Args>
        T* addComponent(Args&&... args)
        {
            // Compile-time safety: ensure T inherits from Component
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

            // Create component with forwarded constructor arguments
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            component->owner = this;  // Set back-reference
            component->onAttach();    // Call lifecycle hook

            T* ptr = component.get(); // Save raw pointer for return

            // Store in map using type as key (allows lookup by type)
            components[std::type_index(typeid(T))] = std::move(component);

            return ptr;
        }

        // ECS: Retrieve a component by type
        // Returns: Pointer to component, or nullptr if not found
        template<typename T>
        T* getComponent()
        {
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

            auto it = components.find(std::type_index(typeid(T)));
            if (it != components.end())
            {
                return static_cast<T*>(it->second.get()); // Downcast to concrete type
            }
            return nullptr;
        }

        // ECS: Check if entity has a specific component type
        template<typename T>
        bool hasComponent() const
        {
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
            return components.find(std::type_index(typeid(T))) != components.end();
        }

        // ECS: Remove a component from this entity
        template<typename T>
        void removeComponent()
        {
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

            auto it = components.find(std::type_index(typeid(T)));
            if (it != components.end())
            {
                it->second->onDetach(); // Call cleanup hook
                components.erase(it);   // Destroy component (unique_ptr)
            }
        }

        // Update all components
        void update(sf::Time dt)
        {
            if (!active) return;

            for (auto& [type, component] : components)
            {
                if (component->active)
                {
                    component->update(dt);
                }
            }
        }

        // Render all components
        void render(sf::RenderWindow& window)
        {
            if (!active) return;

            for (auto& [type, component] : components)
            {
                if (component->active)
                {
                    component->render(window);
                }
            }
        }

        // Getters
        uint64_t getId() const { return id; }
        bool isActive() const { return active; }

        // Setters
        void setActive(bool isActive) { active = isActive; }

        // ENTITY CATEGORIZATION:
        // tag: String identifier for entity type ("Player", "Enemy", "XPGem")
        // layer: Integer for rendering order or collision groups
        std::string tag;
        uint32_t layer = 0;

    private:
        uint64_t id;     // Unique identifier
        bool active;     // Entity can be deactivated without destroying it (pooling)

        // Component storage: type -> component instance
        // std::type_index allows runtime type lookup
        // std::unique_ptr provides automatic memory management
        std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
    };
}
