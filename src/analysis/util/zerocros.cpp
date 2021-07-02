#include "util.h"

std::pair<rpm::vector<double>, rpm::vector<double>>
Analysis::findZerocros(const rpm::vector<double>& y, char m)
{
    const int n = (int) y.size();

    rpm::vector<double> s(n);
    rpm::vector<double> k(n - 1);

    for (int i = 0; i < n; ++i) {
        s[i] = (y[i] >= 0 ? 1.0 : 0.0);
    }

    for (int i = 0; i < n - 1; ++i) {
        k[i] = s[i + 1] - s[i];
    }

    rpm::vector<int> f;
    
    for (int i = 0; i < n - 1; ++i) {
        if ((m == 'p' && k[i] > 0)
                || (m == 'n' && k[i] < 0)
                || (m != 'p' && m != 'n' && k[i] != 0)) {
            f.push_back(i);
        }
    }

    s.resize(f.size());

    for (int i = 0; i < (int) f.size(); ++i) {
        s[i] = y[i + 1] - y[i];
    }

    rpm::vector<double> t(f.size());

    for (int i = 0; i < (int) f.size(); ++i) {
        t[i] = f[i] - y[f[i]] / s[i];
    }

    return std::make_pair(t, s);
}

