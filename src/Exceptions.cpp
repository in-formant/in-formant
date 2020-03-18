//
// Created by clo on 12/09/2019.
//

#include <cstdio>
#include "Exceptions.h"
#include "log/simpleQtLogger.h"

GenericException::GenericException(const char * prefix, const char * msg) {

    message = new char[64];
    sprintf(message, "[%s] %s", prefix, msg);

}

GenericException::~GenericException() {
    delete message;
}
