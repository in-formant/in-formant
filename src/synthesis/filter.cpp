#include "synthesis.h"

std::vector<float> Synthesis::filter(
        const std::vector<float>& b,
        const std::vector<float>& a,
        const std::vector<float>& x, std::deque<float>& memory)
{
    const int length = x.size();
    const int nfilt = a.size();

    std::vector<float> out(length);

    for (int i = 0; i < length; ++i) {
        double val = 0.0;
        for (int j = 0; j < b.size(); ++j) {
            if (i - j >= 0) {
                val += b[j] * x[i - j];
            }
        }
        for (int j = 1; j < nfilt; ++j) {
            val -= (a[j] * memory[j - 1]) / a[0];
        }
        out[i] = val;

        memory.pop_back();
        memory.push_front(val);
    }

    return out;
}

