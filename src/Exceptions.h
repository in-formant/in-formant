//
// Created by clo on 12/09/2019.
//

#ifndef SPEECH_ANALYSIS_EXCEPTIONS_H
#define SPEECH_ANALYSIS_EXCEPTIONS_H


#include <exception>
#include <functional>
#include <string>


class GenericException : public std::exception {
public:
    GenericException(const char * prefix, const char * msg);
    ~GenericException();

    [[nodiscard]]
    const char * what() const noexcept final {
        return message;
    }

private:
    char * message;
};

// Define exceptions.

class AudioException : public GenericException { 
public:
    AudioException(const char * msg) : GenericException("Audio", msg) {}
};

#endif //SPEECH_ANALYSIS_EXCEPTIONS_H
