//
// Created by clo on 12/09/2019.
//

#include <sstream>
#include "Exceptions.h"

GenericException::GenericException(const char * prefix, const char * msg, std::function<const char * ()> error) {
    std::stringstream builder(std::ios_base::out);
    builder << "[" << prefix << "] " << msg << ": " << error();

    message = builder.str();
}
