#pragma once

#include "Hitboxes.hpp"
#include "Color.hpp"


namespace pingame{

class Rect: public RectHitbox{
public:
    Color color;
    std::int16_t z_order;

    Rect(): RectHitbox(0,0,0,0), color(Black), z_order(0){};

    Rect(float x, float y, float w, float h, Color color = Blue): RectHitbox(x, y, w, h), color(color), z_order(0){}

    Rect(Point coord, float w, float h, Color color = Blue): RectHitbox(coord, w, h), color(color), z_order(0){}

    Rect(Point coord, Point size, Color color = Blue): RectHitbox(coord, size), color(color), z_order(0){}

    operator RectHitbox() const{ return RectHitbox(this->pos, this->size * 2); }

    void draw(Scene*) const;
};


class LineRect: public RectHitbox{
public:
    Color color;
    std::uint32_t thickness;
    std::int16_t z_order;

    LineRect(): RectHitbox(0,0,0,0), thickness(0), color(Black), z_order(0){};

    LineRect(float x, float y, float w, float h, Color color = Blue, std::uint32_t thickness = 5): RectHitbox(x, y, w, h), color(color), thickness(thickness), z_order(0){}

    LineRect(Point coord, float w, float h, Color color = Blue, std::uint32_t thickness = 5): RectHitbox(coord, w, h), color(color), thickness(thickness), z_order(0){}

    LineRect(Point coord, Point size, Color color = Blue, std::uint32_t thickness = 5): RectHitbox(coord, size), color(color), thickness(thickness), z_order(0){}

    LineRect(const Rect& rect, std::uint32_t thickness = 5): RectHitbox(rect), color(rect.color), thickness(thickness), z_order(0){}

    operator RectHitbox() const{ return RectHitbox(this->pos, this->size * 2); }

    void draw(Scene*) const;
};


class Circle: public CircleHitbox{
public:
    Color color;
    std::int16_t z_order;

    Circle(): CircleHitbox(0,0,0), color(Black), z_order(0){}

    Circle(float x, float y, float r, Color color = Blue): CircleHitbox(x, y, r), color(color), z_order(0){}

    Circle(Point coord, float r, Color color = Blue): CircleHitbox(coord, r), color(Blue), z_order(0){}

    operator CircleHitbox() const{ return CircleHitbox(this->pos, this->radius); }

    void draw(Scene*) const;
};


class LineCircle: public CircleHitbox{
public:
    Color color;
    std::uint32_t thickness;
    std::int16_t z_order;

    LineCircle(): CircleHitbox(0,0,0), color(Black), thickness(0), z_order(0){}

    LineCircle(float x, float y, float r, Color color = Blue, std::uint32_t thickness = 5): CircleHitbox(x, y, r), color(color), thickness(thickness), z_order(0){}

    LineCircle(Point coord, float r, Color color = Blue, std::uint32_t thickness = 5): CircleHitbox(coord, r), color(Blue), thickness(thickness), z_order(0){}

    LineCircle(const Circle& circle, std::uint32_t thickness = 5): CircleHitbox(circle), color(circle.color), thickness(thickness), z_order(0){};

    operator CircleHitbox() const{ return CircleHitbox(this->pos, this->radius); }

    void draw(Scene*) const;
};


}