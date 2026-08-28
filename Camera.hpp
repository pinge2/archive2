#pragma once

#include "Point.hpp"


namespace pingame{

class Camera{
public:
    Point pos;
    float zoom;

    Camera(float x, float y, float zoom = 1): pos(x, y), zoom(zoom){}

    Camera(Point pos, float zoom = 1): pos(pos), zoom(zoom){}

    Camera(float zoom = 1): pos(0, 0), zoom(zoom){}

    void move(float x, float y){ this->pos.move(x, y); }
    void move(Point coord){ this->pos += coord; }

    Point getWindowCoord(const Window* window, Point coord) const{
        return Point(
            window->getSize().x * 0.5 + (coord.x - this->pos.x) * this->zoom,
            window->getSize().y * 0.5 - (coord.y - this->pos.y) * this->zoom
        );
    }

    Point getCoordFromWindow(const Window* window, Point coord) const{
        return Point(
            this->pos.x + (coord.x - window->getSize().x * 0.5) / this->zoom,
            this->pos.y - (coord.y - window->getSize().y * 0.5) / this->zoom
        );
    }
};

}