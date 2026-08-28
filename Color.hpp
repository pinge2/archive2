#pragma once

#include "Point.hpp"


namespace pingame{

class Color{
public:
    std::uint8_t b, g, r, a;

    constexpr Color(): a(1), r(0), g(0), b(0){}

    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255): a(a), r(r), g(g), b(b){}

    constexpr Color(std::uint8_t c): a(1), r(c), g(c), b(c){}

    constexpr Color(const Color& other): a(other.a), r(other.r), g(other.g), b(other.b){}

    constexpr std::uint32_t bgra() const{ return *(std::uint32_t*)this; }

    constexpr bool operator==(Color other) const{ return this->bgra() == other.bgra(); }

    constexpr Color invert() const{
        return Color(255 - this->r, 255 - this->g, 255 - this->b, 255 - this->a);
    }

    constexpr Color operator+(Color other) const{
        return Color(
            std::clamp(this->r + other.r, 0, 255),
            std::clamp(this->g + other.g, 0, 255),
            std::clamp(this->b + other.b, 0, 255),
            std::clamp(this->a + other.a, 0, 255)
        );
    }

    constexpr Color operator*(float scalar) const{
        return Color(
            std::clamp(int(this->r * scalar), 0, 255),
            std::clamp(int(this->g * scalar), 0, 255),
            std::clamp(int(this->b * scalar), 0, 255),
            std::clamp(int(this->a * scalar), 0, 255)
        );
    }
};


constexpr Color White = Color(255, 255, 255);
constexpr Color Black = Color(0, 0, 0);
constexpr Color Red = Color(255, 0, 0);
constexpr Color Green = Color(0, 255, 0);
constexpr Color Blue = Color(0, 0, 255);
constexpr Color Yellow = Color(255, 255, 0);
constexpr Color Cyan = Color(0, 255, 255);
constexpr Color Magenta = Color(255, 0, 255);
constexpr Color Purple = Color(128, 0, 128);
constexpr Color Orange = Color(255, 165, 0);
constexpr Color Gray = Color(128, 128, 128);
constexpr Color Alpha = Color(0, 0, 0, 0);

}