#include <BufferRenderer.hpp>


namespace pingame{

void BufferRenderer::execute(){
    std::sort(this->buffer.begin(), this->buffer.end());

    for (Command& k: this->buffer){
        this->setColor(k.color);
        debug((int)k.command << " - " << k.rect.x << ", " << k.rect.y << ", " << k.rect.w << ", " << k.rect.h);

        switch (k.command){

        case CommandId::RectDraw:
            SDL_RenderFillRectF(this->renderer, &k.rect);

            break;

        case CommandId::LineRectDraw:
            for (float i = 0.5; i < k.line_thickness; i += 1){
                SDL_RenderDrawRectF(this->renderer, &k.rect);
                k.rect.x += 1;
                k.rect.w -= 2;
                k.rect.y += 1;
                k.rect.h -= 2;
            }

            break;
            
        default: continue;
        }
    }
}

}