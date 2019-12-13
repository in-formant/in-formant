//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_EXCEPTIONS_H
#define SPEECH_ANALYSIS_EXCEPTIONS_H


#include <exception>
#include <functional>
#include <string>
#include <portaudio.h>


class GenericException : public std::exception {
public:
    GenericException(const char * prefix, const char * msg, std::function<const char * ()> error);
    ~GenericException();

    [[nodiscard]]
    const char * what() const noexcept final {
        return message;
    }

private:
    char * message;
};

// Define exceptions.

class PaException : public GenericException {
public:
    PaException(const char * msg, int error) : GenericException("PortAudio", msg, [error]() { return Pa_GetErrorText(error); }) {}
    PaException(const char * msg, const char * error) : GenericException("PortAudio", msg, [error]() { return error; }) {}
};

#endif //SPEECH_ANALYSIS_EXCEPTIONS_H
