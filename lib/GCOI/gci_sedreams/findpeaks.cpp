#include "gci_sedreams.h"

using namespace Eigen;

rpm::vector<int> Sedreams::findPeaks(const ArrayXd & signal, double sign) {
    
    const int N(signal.size());

    int n;

    ArrayXd df1(N);
    df1.tail(N - 1) = signal.tail(N - 1) - signal.head(N - 1);
    df1(0) = df1(1);

    ArrayXd df2(N);
    df2.tail(N - 1) = df1.tail(N - 1) - df1.head(N - 1);
    df2(0) = df2(1) = df2(2);
    df2 *= sign;
    
    // Find sign changes in df1 and negativity in df2
    rpm::vector<int> idx;
    for (n = 0; n < N; ++n) {
        bool df1_sgn = (df1(n) * (n+1 < N ? df1(n+1) : 0.)) <= 0;
        bool df2_neg = (n+1 < N ? df2(n+1) : 0.) < 0;

        if (df1_sgn && df2_neg) {
            idx.push_back(n);
        }
    }

    return idx;
}
