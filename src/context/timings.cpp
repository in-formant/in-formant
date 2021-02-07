#include "timings.h"
#include <iomanip>

namespace timings {
    duration render;
    duration update;
}

std::ostream& operator<<(std::ostream& os, const duration& dur)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(2)
      << dur.count() << " ms";
    return os << s.str();
}
