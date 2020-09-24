#include "synthesis.h"

std::vector<float> Synthesis::filter(
        const float b,
        const std::vector<float>& a,
        const std::vector<float>& x, std::deque<float>& memory)
{
    const int length = x.size();
    const int nfilt = a.size();

    std::vector<float> out(length);

    for (int i = 0; i < length; ++i) {
        double val = b * x[i];
        for (int j = 1; j < nfilt; ++j) {
            val -= (a[j] * memory[j - 1]) / a[0];
        }
        out[i] = val;

        memory.pop_back();
        memory.push_front(val);
    }

    return out;
}

