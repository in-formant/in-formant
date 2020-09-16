#ifndef ANALYSIS_FILTER_H
#define ANALYSIS_FILTER_H

#include <vector>

namespace Analysis {

    std::vector<float> filter(const std::vector<float>& a, const std::vector<float>& x);
    
    std::vector<float> filter(const std::vector<float>& b, const std::vector<float>& a, const std::vector<float>& x);

}

#endif // ANALYSIS_FILTER_H
