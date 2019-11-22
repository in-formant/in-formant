//
// Created by clo on 12/09/2019.
//

#include <cstdio>
#include "Exceptions.h"

GenericException::GenericException(const char * prefix, const char * msg, std::function<const char * ()> error) {

    message = new char[64];

    sprintf(message, "[%s] %s: %s", prefix, msg, error());
}

GenericException::~GenericException() {
    delete message;
}
