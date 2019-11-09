//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_SDLUTILS_H
#define SPEECH_ANALYSIS_SDLUTILS_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>


namespace SDL {

    SDL_Texture * convertAndFreeSurface(SDL_Renderer * renderer, SDL_Surface * surface);

    void queryTextureSize(SDL_Texture * texture, int * width, int * height);

    SDL_Texture * renderText(SDL_Renderer * renderer, TTF_Font * font, const char * text, SDL_Color fg);

    void renderRelativeToCenter(SDL_Renderer * renderer, SDL_Texture * texture, int targetWidth, int targetHeight, const SDL_FPoint * pos);
    void renderRelative(SDL_Renderer * renderer, SDL_Texture * texture, int targetWidth, int targetHeight, const SDL_FPoint * pos);

    void fillTarget(SDL_Renderer * renderer, SDL_Color bgColor);

}


#endif //SPEECH_ANALYSIS_SDLUTILS_H
