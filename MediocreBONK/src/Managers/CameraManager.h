#pragma once
#include <SFML/Graphics.hpp>
#include "../ECS/Entity.h"
#include "../ECS/Components/Transform.h"
#include "../Utils/Math.h"
#include "../Utils/Random.h"

namespace MediocreBONK::Managers
{
    class CameraManager
    {
    public:
        static CameraManager& getInstance()
        {
            static CameraManager instance;
            return instance;
        }

        CameraManager(const CameraManager&) = delete;
        CameraManager& operator=(const CameraManager&) = delete;

        void initialize(const sf::Vector2u& windowSize)
        {
            gameView.setSize(sf::Vector2f(windowSize));
            gameView.setCenter(sf::Vector2f(windowSize) / 2.f);

            uiView.setSize(sf::Vector2f(windowSize));
            uiView.setCenter(sf::Vector2f(windowSize) / 2.f);

            this->windowSize = windowSize;
        }

        void update(sf::Time dt)
        {
            if (!followTarget)
                return;

            auto* transform = followTarget->getComponent<ECS::Components::Transform>();
            if (!transform)
                return;

            // Calculate target position (player position + offset)
            sf::Vector2f targetPos = transform->position + cameraOffset;

            // Smooth lerp to target
            sf::Vector2f currentCenter = gameView.getCenter();
            sf::Vector2f newCenter = Utils::Math::lerp(currentCenter, targetPos, lerpFactor * dt.asSeconds());

            // Apply screen shake if active
            if (shakeTimeRemaining > 0.f)
            {
                shakeTimeRemaining -= dt.asSeconds();

                // Random offset for shake
                float shakeX = Utils::Random::range(-shakeIntensity, shakeIntensity);
                float shakeY = Utils::Random::range(-shakeIntensity, shakeIntensity);

                newCenter += sf::Vector2f(shakeX, shakeY);

                // Decay shake intensity
                shakeIntensity *= 0.95f;
            }

            gameView.setCenter(newCenter);
        }

        void setFollowTarget(ECS::Entity* target)
        {
            followTarget = target;
        }

        void adjustOffset(const sf::Vector2f& adjustment)
        {
            cameraOffset += adjustment;
        }

        void resetOffset()
        {
            cameraOffset = sf::Vector2f(0.f, 0.f);
        }

        void setLerpFactor(float factor)
        {
            lerpFactor = factor;
        }

        void setZoom(float zoom)
        {
            gameView.setSize(sf::Vector2f(windowSize) / zoom);
        }

        void applyScreenShake(float intensity, float duration)
        {
            shakeIntensity = intensity;
            shakeTimeRemaining = duration;
        }

        sf::View& getGameView()
        {
            return gameView;
        }

        sf::View& getUIView()
        {
            return uiView;
        }

        sf::Vector2f getWorldMousePosition(const sf::RenderWindow& window) const
        {
            return window.mapPixelToCoords(sf::Mouse::getPosition(window), gameView);
        }

        // Get half-diagonal of the viewport for spawn calculations
        float getViewHalfDiagonal() const
        {
            sf::Vector2f viewSize = gameView.getSize();
            float halfWidth = viewSize.x / 2.f;
            float halfHeight = viewSize.y / 2.f;
            return std::sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        }

    private:
        CameraManager()
            : followTarget(nullptr)
            , cameraOffset(0.f, 0.f)
            , lerpFactor(5.f)
            , shakeIntensity(0.f)
            , shakeTimeRemaining(0.f)
            , windowSize(1920, 1080)
        {}

        sf::View gameView;
        sf::View uiView;
        ECS::Entity* followTarget;
        sf::Vector2f cameraOffset;
        float lerpFactor;

        // Screen shake
        float shakeIntensity;
        float shakeTimeRemaining;

        sf::Vector2u windowSize;
    };
}
