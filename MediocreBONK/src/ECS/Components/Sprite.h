#pragma once
#include "../Component.h"
#include "Transform.h"
#include "../../Core/ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace MediocreBONK::ECS::Components
{
    class Sprite : public Component
    {
    public:
        Sprite(const std::string& texturePath, int renderLayer = 0)
            : texturePath(texturePath)
            , renderLayer(renderLayer)
            , color(sf::Color::White)
            , texture(&Core::ResourceManager::getInstance().getTexture(texturePath))
            , sprite(*texture)
        {
            // Center origin
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.size / 2.f);
        }

        void render(sf::RenderWindow& window) override
        {
            // Get transform component to position sprite
            if (owner)
            {
                auto* transform = owner->getComponent<Transform>();
                if (transform)
                {
                    sprite.setPosition(transform->position);
                    sprite.setRotation(sf::degrees(transform->rotation));
                    sprite.setScale(transform->scale);
                }
            }

            sprite.setColor(color);
            window.draw(sprite);
        }

        void setTextureRect(const sf::IntRect& rect)
        {
            sprite.setTextureRect(rect);
        }

        void setColor(const sf::Color& newColor)
        {
            color = newColor;
        }

        sf::FloatRect getGlobalBounds() const
        {
            return sprite.getGlobalBounds();
        }

        const sf::Sprite& getSprite() const
        {
            return sprite;
        }

        int renderLayer;

    private:
        std::string texturePath;
        const sf::Texture* texture;
        sf::Sprite sprite;
        sf::Color color;
    };
}
