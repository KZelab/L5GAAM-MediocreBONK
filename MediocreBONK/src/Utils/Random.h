#pragma once
#include <random>
#include <SFML/System/Vector2.hpp>

namespace MediocreBONK::Utils
{
    class Random
    {
    private:
        static std::mt19937& getGenerator()
        {
            static std::random_device rd;
            static std::mt19937 generator(rd());
            return generator;
        }

    public:
        // Random integer in range [min, max]
        static int range(int min, int max)
        {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(getGenerator());
        }

        // Random float in range [min, max]
        static float range(float min, float max)
        {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(getGenerator());
        }

        // Random float in range [0, 1]
        static float value()
        {
            return range(0.0f, 1.0f);
        }

        // Random boolean
        static bool chance(float probability = 0.5f)
        {
            return value() < probability;
        }

        // Random point in circle
        static sf::Vector2f insideCircle(float radius)
        {
            float angle = range(0.0f, 2.0f * 3.14159265f);
            float r = std::sqrt(value()) * radius;
            return sf::Vector2f(std::cos(angle) * r, std::sin(angle) * r);
        }

        // Random point on circle
        static sf::Vector2f onCircle(float radius)
        {
            float angle = range(0.0f, 2.0f * 3.14159265f);
            return sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
        }

        // Random unit direction vector
        static sf::Vector2f direction()
        {
            return onCircle(1.0f);
        }
    };
}
