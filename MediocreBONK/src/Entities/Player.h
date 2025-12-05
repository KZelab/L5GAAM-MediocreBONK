#pragma once
#include "../ECS/Entity.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Sprite.h"
#include "../ECS/Components/Physics.h"
#include "../ECS/Components/Health.h"
#include "../ECS/Components/Weapon.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/Experience.h"
#include "../Utils/Logger.h"
#include <SFML/Window/Keyboard.hpp>

namespace MediocreBONK::Entities
{
    enum class PlayerState
    {
        Idle,
        Walking,
        Dashing
    };

    class Player
    {
    public:
        Player(ECS::Entity* entity, const sf::Vector2f& startPosition)
            : entity(entity)
            , moveSpeed(500)
            , dashSpeed(2000.f)
            , dashDuration(0.2f)
            , dashCooldown(1.f)
            , state(PlayerState::Idle)
            , facingRight(true)
            , dashTimer(0.f)
            , dashCooldownTimer(0.f)
            , invulnerabilityTimer(0.f)
        {
            // Add components
            transform = entity->addComponent<ECS::Components::Transform>(startPosition);
            // sprite = entity->addComponent<ECS::Components::Sprite>("assets/sprites/player.png", 1);
            physics = entity->addComponent<ECS::Components::Physics>();
            health = entity->addComponent<ECS::Components::Health>(100.f);

            // Add collider (circle shape)
            entity->addComponent<ECS::Components::Collider>(ECS::Components::ColliderShape::Circle, 20.f);

            // Add basic weapon
            ECS::Components::WeaponData weaponData;
            weaponData.name = "Basic Shot";
            weaponData.damage = 10.f;
            weaponData.fireRate = 4.f; // 3 shots per second
            weaponData.projectileSpeed = 500.f;
            weaponData.piercing = 1;
            weaponData.projectileCount = 1;
            weaponData.spread = 0.f;
            weaponData.range = 1300.f; // Increased to reach enemies at spawn distance (~1151px)
            weaponData.projectileSprite = "assets/sprites/projectile.png";

            auto* weapon = entity->addComponent<ECS::Components::Weapon>(weaponData);
            weapon->autoFire = true; // Enable auto-fire

            // Add experience component
            auto* experience = entity->addComponent<ECS::Components::Experience>(1);
            experience->onLevelUpCallback = [this](int level) {
                onLevelUp(level);
            };

            physics->drag = 0.85f;

            entity->tag = "Player";
        }

        void update(sf::Time dt)
        {
            float deltaSeconds = dt.asSeconds();

            // Update timers
            if (dashTimer > 0.f)
                dashTimer -= deltaSeconds;

            if (dashCooldownTimer > 0.f)
                dashCooldownTimer -= deltaSeconds;

            if (invulnerabilityTimer > 0.f)
            {
                invulnerabilityTimer -= deltaSeconds;
                health->invulnerable = (invulnerabilityTimer > 0.f);
            }

            // Handle state machine
            switch (state)
            {
            case PlayerState::Dashing:
                updateDashing(deltaSeconds);
                break;
            default:
                updateNormalMovement(deltaSeconds);
                break;
            }

            // Update state based on velocity
            updateState();
        }

        void handleInput()
        {
            // Can't control during dash
            if (state == PlayerState::Dashing)
                return;

            // Horizontal movement (8-direction)
            sf::Vector2f moveInput(0.f, 0.f);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                moveInput.x -= 1.f;
                facingRight = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                moveInput.x += 1.f;
                facingRight = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                moveInput.y -= 1.f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                moveInput.y += 1.f;
            }

            // Normalize diagonal movement
            float magnitude = std::sqrt(moveInput.x * moveInput.x + moveInput.y * moveInput.y);
            if (magnitude > 0.f)
            {
                moveInput /= magnitude;
            }

            // Apply movement force (only horizontal, vertical is for top-down aspect)
            physics->applyForce(moveInput * moveSpeed);
        }



        void dash()
        {
            if (dashCooldownTimer > 0.f || state == PlayerState::Dashing)
                return;

            // Start dash
            state = PlayerState::Dashing;
            dashTimer = dashDuration;
            dashCooldownTimer = dashCooldown;

            // Set invulnerability (0.2 seconds as decided)
            invulnerabilityTimer = 0.2f;
            health->invulnerable = true;

            // Dash in facing direction
            sf::Vector2f dashDirection = facingRight ? sf::Vector2f(1.f, 0.f) : sf::Vector2f(-1.f, 0.f);
            physics->velocity = dashDirection * dashSpeed;
        }

        PlayerState getState() const { return state; }
        bool isInvulnerable() const { return invulnerabilityTimer > 0.f; }
        ECS::Entity* getEntity() { return entity; }

        void onLevelUp(int newLevel)
        {
            // Level up callback - will trigger GUI in GameState
            Utils::Logger::info("Player leveled up to level " + std::to_string(newLevel));
            // Set flag for GameState to show level-up menu
            levelUpPending = true;
        }

        bool hasLevelUpPending() const { return levelUpPending; }
        void clearLevelUpPending() { levelUpPending = false; }

    private:
        void updateNormalMovement(float dt)
        {
            // Normal movement physics handled by Physics component
        }

        void updateDashing(float dt)
        {
            if (dashTimer <= 0.f)
            {
                // End dash
                state = PlayerState::Idle;
                physics->velocity *= 0.5f; // Slow down after dash
            }
        }



        void updateState()
        {
            if (state == PlayerState::Dashing)
                return;

            float speed = std::sqrt(physics->velocity.x * physics->velocity.x + physics->velocity.y * physics->velocity.y);

            if (speed > 10.f)
                state = PlayerState::Walking;
            else
                state = PlayerState::Idle;
        }

        ECS::Entity* entity;
        ECS::Components::Transform* transform;
        ECS::Components::Sprite* sprite;
        ECS::Components::Physics* physics;
        ECS::Components::Health* health;

        // Movement parameters
        float moveSpeed;
        float dashSpeed;
        float dashDuration;
        float dashCooldown;

        // State
        PlayerState state;
        bool facingRight;

        // Timers
        float dashTimer;
        float dashCooldownTimer;
        float invulnerabilityTimer;

        // Level up state
        bool levelUpPending = false;
    };
}
