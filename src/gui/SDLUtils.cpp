//
// Created by clo on 12/09/2019.
//

#include "../Exceptions.h"
#include "SDLUtils.h"

SDL_Texture * SDL::convertAndFreeSurface(SDL_Renderer * renderer, SDL_Surface * surface) {

    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == nullptr) {
        throw SDLException("Unable to create texture from surface");
    }

    SDL_FreeSurface(surface);
    return texture;

}

void SDL::queryTextureSize(SDL_Texture * texture, int * width, int * height) {
    int ret;

    ret = SDL_QueryTexture(texture, nullptr, nullptr, width, height);
    if (ret < 0) {
        throw SDLException("Unable to query texture size");
    }
}

SDL_Texture * SDL::renderText(SDL_Renderer * renderer, TTF_Font * font, const char * text, SDL_Color fg) {

    SDL_Surface * surface = TTF_RenderUTF8_Blended(font, text, fg);

    if (surface == nullptr) {
        throw TTFException("Unable to render UTF8 blended text");
    }

    return SDL::convertAndFreeSurface(renderer, surface);

}

void SDL::renderRelativeToCenter(SDL_Renderer * renderer, SDL_Texture * texture, int targetWidth, int targetHeight, const SDL_FPoint * pos) {

    int ret;

    SDL_Rect dstRect;

    SDL::queryTextureSize(texture, &dstRect.w, &dstRect.h);
    dstRect.x = static_cast<int>(pos->x * static_cast<float>(targetWidth - dstRect.w));
    dstRect.y = static_cast<int>(pos->y * static_cast<float>(targetHeight - dstRect.h));

    ret = SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    if (ret < 0) {
        throw SDLException("Unable to render texture");
    }

}

void SDL::renderRelative(SDL_Renderer * renderer, SDL_Texture * texture, int targetWidth, int targetHeight, const SDL_FPoint * pos) {

    int ret;

    SDL_Rect dstRect;

    SDL::queryTextureSize(texture, &dstRect.w, &dstRect.h);
    dstRect.x = static_cast<int>(pos->x * static_cast<float>(targetWidth));
    dstRect.y = static_cast<int>(pos->y * static_cast<float>(targetHeight));

    ret = SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    if (ret < 0) {
        throw SDLException("Unable to render texture");
    }

}

void SDL::fillTarget(SDL_Renderer * renderer, SDL_Color bgColor) {

    // Save the current render draw color.
    Uint8 r, g, b, a;

    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

    // Set draw color to bg color and fill the entire target.
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, nullptr);

    // Restore the previous draw color.
    SDL_SetRenderDrawColor(renderer, r, g, b, a);


}