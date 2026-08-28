#include "Scene.hpp"
#include "Window.hpp"


namespace pingame{

bool Scene::is_pressed(const char* code) const{
    SDL_Scancode scancode = SDL_GetScancodeFromName(code);

    return this->window->keys[scancode] != 0;
}


bool Scene::is_released(const char* code) const{
    SDL_Scancode scancode = SDL_GetScancodeFromName(code);

    return this->window->keys[scancode] == 0;
}


bool Scene::is_pressed_once(const char* code) const{
    SDL_Scancode scancode = SDL_GetScancodeFromName(code);

    return this->window->getHeldKey() == scancode;
}


bool Scene::is_released_once(const char* code) const{
    SDL_Scancode scancode = SDL_GetScancodeFromName(code);

    return this->window->getUnheldKey() == scancode;
}


bool Scene::is_left_pressed() const{
    return this->window->mouse & SDL_BUTTON_LEFT;
}


bool Scene::is_right_pressed() const{
    return this->window->mouse & SDL_BUTTON_RIGHT;
}


bool Scene::is_middle_pressed() const{
    return this->window->mouse & SDL_BUTTON_MIDDLE;
}

}