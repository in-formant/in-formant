#include "gammatonegram.h"

using namespace std::literals::complex_literals;

rpm::vector<rpm::vector<std::array<double, 6>>> Analysis::makeErbFilters(double fs, int numChannels, double lowFreq)
{
    using namespace ERB;
    const double T = 1 / fs;
    Eigen::ArrayXd cf = erbSpace(lowFreq, fs / 2, numChannels);
    Eigen::ArrayXd ERB = pow(pow(cf / EarQ, order) + pow(minBW, order), 1 / order);
    Eigen::ArrayXd B = 1.019 * 2 * M_PI * ERB;

    double A0 = T;
    double A2 = 0;
    double B0 = 1;
    Eigen::ArrayXd B1 = -2 * cos(2 * cf * M_PI * T) / exp(B * T);
    Eigen::ArrayXd B2 = exp(-2 * B * T);

    Eigen::ArrayXd A11 = -(2 * T * cos(2 * cf * M_PI * T) / exp(B * T) +
                            2 * sqrt(3 + pow(2, 1.5)) * T * sin(2 * cf * M_PI * T) /
                            exp(B * T)) / 2;
    Eigen::ArrayXd A12 = -(2 * T * cos(2 * cf * M_PI * T) / exp(B * T) -
                            2 * sqrt(3 + pow(2, 1.5)) * T * sin(2 * cf * M_PI * T) /
                            exp(B * T)) / 2;
    Eigen::ArrayXd A13 = -(2 * T * cos(2 * cf * M_PI * T) / exp(B * T) +
                            2 * sqrt(3 - pow(2, 1.5)) * T * sin(2 * cf * M_PI * T) /
                            exp(B * T)) / 2;
    Eigen::ArrayXd A14 = -(2 * T * cos(2 * cf * M_PI * T) / exp(B * T) -
                            2 * sqrt(3 - pow(2, 1.5)) * T * sin(2 * cf * M_PI * T) /
                            exp(B * T)) / 2;

    Eigen::ArrayXd gain = abs((-2 * exp(4.0 * 1.0i * cf * M_PI * T) * T +
                                  2 * exp(-(B * T) + 2.0 * 1.0i * cf * M_PI * T) * T *
                                    (cos(2 * cf * M_PI * T) - sqrt(3 - pow(2, 1.5)) *
                                     sin(2 * cf * M_PI * T))) *
                                (-2 * exp(4.0 * 1.0i * cf * M_PI * T) * T +
                                  2 * exp(-(B * T) + 2.0 * 1.0i * cf * M_PI * T) * T *
                                    (cos(2 * cf * M_PI * T) + sqrt(3 - pow(2, 1.5)) *
                                     sin(2 * cf * M_PI * T))) *
                                (-2 * exp(4.0 * 1.0i * cf * M_PI * T) * T +
                                  2 * exp(-(B * T) + 2.0 * 1.0i * cf * M_PI * T) * T *
                                    (cos(2 * cf * M_PI * T) + sqrt(3 + pow(2, 1.5)) *
                                     sin(2 * cf * M_PI * T))) /
                                pow(-2 / exp(2 * B * T) - 2 * exp(4.0 * 1.0i * cf * M_PI * T) +
                                  2 * (1 + exp(4.0 * 1.0i * cf * M_PI * T)) / exp(B * T), 4));

    rpm::vector<rpm::vector<std::array<double, 6>>> filters;
    for (int i = 0; i < cf.size(); ++i) {
        rpm::vector<std::array<double, 6>> filter;
        filter.push_back({A0/gain(i), A11(i)/gain(i), A2/gain(i), B0, B1(i), B2(i)});
        filter.push_back({A0, A12(i), A2, B0, B1(i), B2(i)});
        filter.push_back({A0, A13(i), A2, B0, B1(i), B2(i)});
        filter.push_back({A0, A14(i), A2, B0, B1(i), B2(i)});
        filters.push_back(std::move(filter));
    }
    return filters;
}
