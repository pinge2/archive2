#pragma once

#include "BufferRenderer.hpp"


namespace pingame{

class Scene;

class Window{
public:
    const std::uint8_t* keys;
    std::uint32_t mouse;
    BufferRenderer renderer;

    Window(const std::string&, std::uint16_t, std::uint16_t, bool);

    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    std::string getTitle() const{ return this->title; }

    Point getSize() const{ return Point(float(this->width), float(this->height));}

    bool isFullscreen() const{ return this->is_fullscreen; }

    std::uint16_t getFps() const{ return this->fps; }

    bool isVSync() const{ return this->is_vsync; }
    
    bool isClosed() const{ return this->is_closed; }

    bool isResizable() const{ return this->is_resizable; }

    Point mouseScreenPos() const{ return Point(float(this->mouse_x), float(this->mouse_y)); };

    void setBgColor(Color color){ this->background = color; }

    SDL_Scancode getHeldKey() const{ return this->held_key; };

    SDL_Scancode getUnheldKey() const{ return this->unheld_key; };

    void setTitle(const std::string&) noexcept;
    void setSize(Point) noexcept;
    void setFullscreen(bool) noexcept;
    void setFps(std::uint16_t) noexcept;
    void setVSync(bool) noexcept;
    void setResizable(bool) noexcept;

    void setScene(Scene*) noexcept;

    void close() noexcept;
    void run() noexcept;


private:
    std::uint16_t fps;
    bool is_fullscreen, is_closed;
    bool is_vsync, is_resizable;
    std::int32_t width, height;
    Color background;
    std::int32_t mouse_x, mouse_y;
    std::string title;
    SDL_Scancode held_key, unheld_key;
    
    Scene* scene;
    SDL_Window* window;
};

}