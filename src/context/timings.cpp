#include "timings.h"
#include <iomanip>
#include <sstream>

namespace timings {
    duration render;
    duration update;

    duration updateSpectrogram;
    duration updatePitch;
    duration updateFormants;
    duration updateOscilloscope;
}

std::ostream& operator<<(std::ostream& os, const duration& dur)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(2)
      << dur.count() << " ms";
    return os << s.str();
}
