#pragma once

#include "Paint.hpp"


namespace pingame{

template<class Object>
class Group{
public:
    std::vector<Object> objects;

    Group(){ this->objects.reserve(32); }

    Group(std::initializer_list<Object> list): objects(list){}

    void append(Object obj){ this->objects.push_back(obj); }

    void remove(Object obj){
        for (int s = 0; s < this->objects.size(); ++s){
            if (this->objects[s] == obj) this->objects.erase(this->objects.cbegin() + s);
        }
    }

    void remove(int idx){ this->objects.erase(this->objects.cbegin() + idx); }

    void clear(){ this->objects.clear(); }

    void move(Point coord){
        for (int s = 0; s < this->objects.size(); ++s) this->objects[i].move(coord);
    }

    void move(float x, float y){ this->move(Point(x, y)); }

    void draw(const Scene* scene){
        for (Object& obj: this->objects) obj.draw(scene);
    }

    bool collides(const RectHitbox& rect){
        for (Object& obj: this->objects){
            if (obj.collides(rect)) return true;
        }

        return false
    }

    bool collides(const CircleHitbox& circle){
        for (Object& obj: this->objects){
            if (obj.collides(circle)) return true;
        }

        return false
    }

    template<class Other>
    bool collides(const Group<Other>& group){
        for (Object& obj: this->objects){
            if (group.collides(obj)) return true;
        }

        return false
    }
};

}