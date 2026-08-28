#pragma once

#include "Camera.hpp"


namespace pingame{

class Window;

class Scene{
public:
    Window* window;
    Camera camera;

    Scene(): window(nullptr){}
    Scene(Window* window): window(window){}

    virtual ~Scene(){};

    bool is_pressed(const char*) const noexcept;

    bool is_released(const char*) const noexcept;

    bool is_pressed_once(const char*) const noexcept;

    bool is_released_once(const char*) const noexcept;

    bool is_left_pressed() const noexcept;

    bool is_right_pressed() const noexcept;

    bool is_middle_pressed() const noexcept;

    Point mousePos() const{ return this->camera.getCoordFromWindow(this->window, this->window->mouseScreenPos()); }

    virtual void update(float){};
    virtual void render(){};
};

}