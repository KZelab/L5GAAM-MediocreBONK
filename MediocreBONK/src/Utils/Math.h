#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace MediocreBONK::Utils
{
    class Math
    {
    public:
        // Calculate magnitude of a vector
        static float magnitude(const sf::Vector2f& vec)
        {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y);
        }

        // Normalize a vector
        static sf::Vector2f normalize(const sf::Vector2f& vec)
        {
            float mag = magnitude(vec);
            if (mag == 0.0f)
                return sf::Vector2f(0.0f, 0.0f);
            return vec / mag;
        }

        // Distance between two points
        static float distance(const sf::Vector2f& a, const sf::Vector2f& b)
        {
            return magnitude(b - a);
        }

        // Distance squared (faster than distance, avoids sqrt)
        static float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b)
        {
            sf::Vector2f diff = b - a;
            return diff.x * diff.x + diff.y * diff.y;
        }

        // Linear interpolation
        static float lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        // Vector lerp
        static sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t)
        {
            return sf::Vector2f(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
        }

        // Clamp value between min and max
        static float clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        // Dot product
        static float dot(const sf::Vector2f& a, const sf::Vector2f& b)
        {
            return a.x * b.x + a.y * b.y;
        }

        // Convert degrees to radians
        static float toRadians(float degrees)
        {
            return degrees * 3.14159265f / 180.0f;
        }

        // Convert radians to degrees
        static float toDegrees(float radians)
        {
            return radians * 180.0f / 3.14159265f;
        }
    };
}
