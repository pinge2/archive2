#include <pingame.hpp>


namespace pingame{

Window::Window(const std::string& title, std::uint16_t width, std::uint16_t height, bool fullscreen){
    this->title = title;
    this->width = width;
    this->height = height;
    this->background = Black;
    this->is_fullscreen = fullscreen;
    this->is_closed = true;
    this->fps = 60;
    this->is_vsync = false;
    this->is_resizable = true;
    this->scene = new Scene(this);
    
    SDL_Init(SDL_INIT_EVERYTHING);

    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
    
    this->window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    this->renderer = BufferRenderer(this->window);
}


Window::~Window(){
    this->close();

    SDL_Quit();
}


void Window::setTitle(const std::string& title){
    this->title = title;

    SDL_SetWindowTitle(this->window, title.c_str());
}


void Window::setSize(Point size){
    this->width = std::uint16_t(size.x);
    this->height = std::uint16_t(size.y);

    SDL_SetWindowSize(this->window, width, height);
}


void Window::setFullscreen(bool fullscreen){
    this->is_fullscreen = fullscreen;

    SDL_SetWindowFullscreen(this->window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}


void Window::setFps(std::uint16_t fps = 0){
    this->fps = fps;
}


void Window::setResizable(bool resizable){
    this->is_resizable = resizable;

    SDL_SetWindowResizable(this->window, resizable ? SDL_TRUE : SDL_FALSE);
}


void Window::setVSync(bool vsync){
    this->is_vsync = vsync;

    this->renderer.setVSync(vsync);
}


void Window::setScene(Scene* scene){
    delete this->scene;
    this->scene = scene;
}



void Window::close(){
    this->is_closed = true;

    SDL_DestroyWindow(this->window);
}


void Window::run(){
    SDL_Event ev;
    
    this->is_closed = false;

    this->renderer.setBlending(true);

    while (!this->is_closed){
        uint64_t start = SDL_GetTicks64();

        this->renderer.clearBuffer(this->background);

        while (SDL_PollEvent(&ev)){
            if (ev.type == SDL_KEYDOWN) this->held_key = ev.key.keysym.scancode;
            else this->held_key = SDL_SCANCODE_UNKNOWN;

            if (ev.type == SDL_KEYUP) this->unheld_key = ev.key.keysym.scancode;
            else this->unheld_key = SDL_SCANCODE_UNKNOWN;

            if (ev.type == SDL_QUIT) this->close();

            else if (ev.type == SDL_WINDOWEVENT){
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED) SDL_GetWindowSize(this->window, &this->width, &this->height);
            }
        }

        this->keys = SDL_GetKeyboardState(0);
        this->mouse = SDL_GetMouseState(&this->mouse_x, &this->mouse_y);
    
        this->scene->render();

        this->renderer.execute();
        this->renderer.present();

        if (!this->is_vsync && this->fps > 0){
            uint64_t needed = 1000 / this->fps;
            uint64_t frametime = SDL_GetTicks64() - start;

            if (needed > frametime) SDL_Delay(needed - frametime);
        }
    }
}

}