#pragma once

#include "types.hpp"


namespace pingame{

class Point{
public:
    float x, y;

    Point(float x, float y): x(x), y(y){}

    Point(float coord = 0): x(coord), y(coord){}

    void move(float x, float y){ this->x += x; this->y += y; }

    Point operator+(Point other) const{
        return Point(this->x + other.x, this->y + other.y);
    }

    Point operator-(Point other) const{
        return Point(this->x - other.x, this->y - other.y);
    }

    Point operator*(Point other) const{
        return Point(this->x * other.x, this->y * other.y);
    }

    Point operator/(Point other) const{
        return Point(this->x / other.x, this->y / other.y);
    }

    Point operator+(float coord) const{ return *this + Point(coord); }
    Point operator-(float coord) const{ return *this - Point(coord); }
    Point operator*(float coord) const{ return *this * Point(coord); }
    Point operator/(float coord) const{ return *this / Point(coord); }
    
    void operator+=(Point other){ *this = *this + other; }
    void operator-=(Point other){ *this = *this - other; }
    void operator*=(Point other){ *this = *this * other; }
    void operator/=(Point other){ *this = *this / other; }
    void operator+=(float coord){ *this = *this + coord; }
    void operator-=(float coord){ *this = *this - coord; }
    void operator*=(float coord){ *this = *this * coord; }
    void operator/=(float coord){ *this = *this / coord; }

    bool operator>(Point other) const{ return this->x > other.x && this->y > other.y; }
    bool operator<(Point other) const{ return this->x < other.x && this->y < other.y; }
    bool operator==(Point other) const{ return this->x == other.x && this->y == other.y; }
    
    bool operator>=(Point other) const{ return *this > other || *this == other; }
    bool operator<=(Point other) const{ return *this < other || *this == other; }

    Point abs() const{ return Point(std::abs(this->x), std::abs(this->y)); }

    float dot(Point other) const{ return this->x * other.x + this->y * other.y; }

    float dot() const{ return this->x * this->x + this->y * this->y; }

    float distSq(Point other) const{
        float dx = other.x - this->x;
        float dy = other.y - this->y;

        return dx * dx + dy * dy;
    }

    float dist(Point other) const{ return std::sqrtf(this->distSq(other)); }
};

}