#include <climits>
#include <cmath>
#include "../../Signal/Filter.h"
#include "../../Signal/Window.h"
#include "../../LPC/LPC.h"
#include "gci_sedreams.h"

using namespace Eigen;

static constexpr int hpfilt = 20;

std::pair<std::vector<int>, std::vector<int>> Sedreams::gci_sedreams(const ArrayXd & signal, const double fs, const double T0mean) {
    const int N(signal.size());
    int n;

    ArrayXd res = LPC::residual(
            signal,
            round(25. / 1000. * fs),
            round(5. / 1000. * fs),
            24
    );

    // filter out any NaNs
    for (n = 0; n < N; ++n) {
        if (!std::isnormal(res(n))) {
            res(n) = 0.;
        }
    }

    // Calculation of the mean-base signal
    int halfL = std::min<int>(round((1.75 * T0mean * fs) / 2.), N / 3);
    ArrayXd blackwin = Window::createBlackmanHarris(2 * halfL + 1) / (double) (2 * halfL + 1);
    
    // Filter wave with blackwin and take mean
    ArrayXd unshiftedMeanBasedSignal;
    Filter::apply(blackwin, signal, unshiftedMeanBasedSignal);

    ArrayXd meanBasedSignal(N);
    meanBasedSignal(seq(halfL, last - halfL)) = unshiftedMeanBasedSignal(seq(2 * halfL, last));
    meanBasedSignal.head(halfL).setZero();
    meanBasedSignal.tail(halfL).setZero();

    // Remove low frequency contents  TODO:ellipsis IIR fiter
    for (n = 0; n < hpfilt; ++n) {
        Filter::preEmphasis(meanBasedSignal, fs, 50.0);
    }
    meanBasedSignal /= meanBasedSignal.abs().maxCoeff();

    // Detect minima and maxima of the mean-based signal
    auto maxInd = findPeaks(meanBasedSignal, 1.);
    auto minInd = findPeaks(meanBasedSignal, -1.);
   
    std::vector<int> gci;

    for (int start : minInd) {
        int end = start + std::round(0.25 * fs * T0mean);
        
        start = std::max(start, 0);
        end = std::min(end, N - 1);

        int maxResInd = start;
        double maxResVal = std::numeric_limits<double>::lowest();

        for (int i = start; i <= end; ++i) {
            if (res(i) > 0.4 && res(i) > maxResVal) {
                maxResInd = i;
                maxResVal = res(i);
            }
        }

        gci.push_back(maxResInd);
    }
     
    std::vector<int> goi;

    for (int start : maxInd) {
        int end = start + std::round(0.15 * fs * T0mean);
        
        start = std::max(start, 0);
        end = std::min(end, N - 1);

        int maxResInd = start;
        double maxResVal = std::numeric_limits<double>::lowest();

        for (int i = start; i <= end; ++i) {
            if (res(i) > 0.2 && res(i) > maxResVal) {
                maxResInd = i;
                maxResVal = res(i);
            }
        }

        goi.push_back(maxResInd);
    }

    return std::make_pair(std::move(gci), std::move(goi));

}
