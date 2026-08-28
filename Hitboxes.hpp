#pragma once

#include "Window.hpp"
#include "Scene.hpp"


namespace pingame{

class CircleHitbox;
class RectHitbox;


class CircleHitbox{
public:
    Point pos;
    float radius;

    CircleHitbox(float x, float y, float r): pos(x, y), radius(r){};

    CircleHitbox(Point coord, float r): pos(coord), radius(r){};

    CircleHitbox(const RectHitbox&);

    void move(float x, float y){ this->pos.move(x, y); }
    void scale(float scale){ this->radius *= scale; }
    void move(Point coord){ this->pos += coord; }

    bool collides(const RectHitbox&) const;
    bool collides(const CircleHitbox&) const;

    bool contains(Point) const;
    
    bool is_hovered(const Scene* scene) const{ return this->contains(scene->mousePos()); };

    bool is_left_pressed(const Scene* scene) const noexcept;

    bool is_right_pressed(const Scene* scene) const noexcept;
};


class RectHitbox{
public:
    Point pos, size;

    RectHitbox(float x, float y, float w, float h): pos(x, y), size(w * 0.5, h * 0.5){};
    RectHitbox(float x, float y, float wh): pos(x, y), size(wh * 0.5, wh * 0.5){};

    RectHitbox(Point coord, float w, float h): pos(coord), size(w * 0.5, h * 0.5){};
    RectHitbox(Point coord, Point size): pos(coord), size(size * 0.5){};

    RectHitbox(const CircleHitbox&);

    void move(float x, float y){ this->pos.move(x, y); }
    void scale(float scale){ this->size *= scale; }
    void move(Point coord){ this->pos += coord; }

    bool collides(const RectHitbox&) const;
    bool collides(const CircleHitbox&) const;

    bool contains(Point) const;

    bool is_hovered(const Scene* scene) const{ return this->contains(scene->mousePos()); };

    bool is_left_pressed(const Scene* scene) const noexcept;

    bool is_right_pressed(const Scene* scene) const noexcept;
};




}