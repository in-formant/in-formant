//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_EXCEPTIONS_H
#define SPEECH_ANALYSIS_EXCEPTIONS_H


#include <exception>
#include <functional>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <soundio/soundio.h>


class GenericException : public std::exception {
public:
    GenericException(const char * prefix, const char * msg, std::function<const char * ()> error);

    [[nodiscard]]
    const char * what() const noexcept final {
        return message.c_str();
    }

private:
    std::string message;
};

// Define exceptions.

class SDLException : public GenericException {
public:
    SDLException(const char * msg) : GenericException("SDL", msg, SDL_GetError) {}
};

class TTFException : public GenericException {
public:
    TTFException(const char * msg) : GenericException("TTF", msg, TTF_GetError) {}
};

class SioException : public GenericException {
public:
    SioException(const char * msg, int error) : GenericException("Sio", msg, [error]() { return soundio_strerror(error); }) {}
    SioException(const char * msg, const char * error) : GenericException("Sio", msg, [error]() { return error; }) {}

};

#endif //SPEECH_ANALYSIS_EXCEPTIONS_H
