#pragma once

#include "Color.hpp"


namespace pingame{

enum class CommandId: std::uint8_t{
    None = 0,
    RectDraw, LineRectDraw,
    CircleDraw, LineCircleDraw,
    TextureDraw
};


struct Command{
    CommandId command;
    std::uint16_t line_thickness;
    std::int32_t z_order;
    Color color;
    const SDL_Texture* texture;
    SDL_FRect rect;

    bool operator>(const Command& other) const{ return this->z_order > other.z_order; }
    bool operator<(const Command& other) const{ return this->z_order < other.z_order; }
};


class BufferRenderer{
public:
    BufferRenderer(SDL_Window* window = nullptr){
        this->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        this->buffer.reserve(256);
    }

    ~BufferRenderer(){
        SDL_DestroyRenderer(this->renderer);
    }

    void setVSync(bool vsync){
        SDL_RenderSetVSync(this->renderer, vsync ? SDL_TRUE : SDL_FALSE);
    }

    void setColor(Color color){
        SDL_SetRenderDrawColor(this->renderer, color.r, color.g, color.b, color.a);
    }

    void setBlending(bool blend){
        SDL_SetRenderDrawBlendMode(this->renderer, blend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    }

    void sendCommand(Command cmd){
        this->buffer.push_back(cmd);
    }

    void clearBuffer(Color color){
        this->setColor(color);
        SDL_RenderClear(this->renderer);

        this->buffer.clear();
    }

    void present(){
        SDL_RenderPresent(this->renderer);
    }

    void execute() noexcept;


private:
    std::vector<Command> buffer;
    SDL_Renderer* renderer;
};

}