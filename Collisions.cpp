#include <Hitboxes.hpp>


namespace pingame{

CircleHitbox::CircleHitbox(const RectHitbox& rect){
    this->pos = rect.pos;
    this->radius = std::max(rect.size.x, rect.size.y);
}


RectHitbox::RectHitbox(const CircleHitbox& circle){
    this->pos = circle.pos - circle.radius;
    this->size = Point(circle.radius);
}


bool CircleHitbox::contains(Point coord) const{
    float dist = (coord - this->pos).dot();

    return dist <= radius * radius;
}


bool RectHitbox::contains(Point coord) const{
    Point min = this->pos - this->size;
    Point max = this->pos + this->size;
    
    return coord >= min && coord <= max;
}


bool CircleHitbox::collides(const CircleHitbox& other) const{
    float dist = (other.pos - this->pos).dot();

    float r = this->radius + other.radius;
    
    return dist <= r * r;
}


bool CircleHitbox::collides(const RectHitbox& other) const{
    Point closest = std::max(other.pos, std::min(this->pos, other.pos + other.size));

    float dist = (this->pos - closest).dot();
    float r = this->radius * this->radius;

    return dist <= r;
}


bool RectHitbox::collides(const RectHitbox& other) const{
    Point dist_abs = (other.pos - this->pos).abs();

    Point size_sum = this->size + other.size;

    return dist_abs.x < size_sum.x && dist_abs.y < size_sum.y;
}


bool RectHitbox::collides(const CircleHitbox& other) const{
    return other.collides(*this);
}


bool RectHitbox::is_left_pressed(const Scene* scene) const{
    return this->is_hovered(scene) && scene->is_left_pressed();
}


bool RectHitbox::is_right_pressed(const Scene* scene) const{
    return this->is_hovered(scene) && scene->is_right_pressed();
}


bool CircleHitbox::is_left_pressed(const Scene* scene) const{
    return this->is_hovered(scene) && scene->is_left_pressed();
}


bool CircleHitbox::is_right_pressed(const Scene* scene) const{
    return this->is_hovered(scene) && scene->is_right_pressed();
}


};