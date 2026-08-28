#include <Paint.hpp>


namespace pingame{

void Rect::draw(Scene* scene) const{
    Point p1 = scene->camera.getWindowCoord(scene->window, this->pos - this->size);
    Point p2 = scene->camera.getWindowCoord(scene->window, this->pos + this->size) - p1;

    Command cmd {
        CommandId::RectDraw,
        0,
        this->z_order,
        this->color,
        nullptr,
        {p1.x, p1.y, p2.x, p2.y}
    };

    scene->window->renderer.sendCommand(cmd);
}


void LineRect::draw(Scene* scene) const{
    Point p1 = scene->camera.getWindowCoord(scene->window, this->pos - this->size);
    Point p2 = scene->camera.getWindowCoord(scene->window, this->pos + this->size) - p1;

    Command cmd {
        CommandId::LineRectDraw,
        std::uint16_t(this->thickness * scene->camera.zoom),
        this->z_order,
        this->color,
        nullptr,
        {p1.x, p1.y, p2.x, p2.y}
    };

    scene->window->renderer.sendCommand(cmd);
}


void Circle::draw(Scene* scene) const{}

}