//
// Created by rika on 12/10/2019.
//

#include "Frame/LPC_Frame.h"
#include "LPC_huber.h"

using namespace Eigen;
using LPC::Huber::huber_s;

static double H_quantile(const ArrayXd & a, double factor);
static void H_mad(const ArrayXd & x, double * inout_location, bool wantLocation, double * out_mad);
static double H_gaussP(double z);
static double H_gauss(double x);
static void H_clip(ArrayXd & x, double min, double max);

double H_quantile(const ArrayXd & a, double factor)
{
    double place = factor * a.size() + 0.5;
    int left = std::floor(place);
    if (a.size() < 1) return 0.0;
    if (a.size() == 1) return a(0);
    if (left < 0) left = 0;
    if (left >= a.size() - 1) left = a.size() - 2;
    if (a(left + 1) == a(left)) return a(left);
    return a(left) + (place - left) * (a(left + 1) - a(left));
}

void H_mad(const ArrayXd & x, double * inout_location, bool wantLocation, double * out_mad)
{
    if (x.size() == 1) {
        if (wantLocation) *inout_location = x(0);
        if (out_mad != nullptr) *out_mad = NAN;
        return;
    }

    ArrayXd work = x;

    if (wantLocation) {
        std::sort(work.begin(), work.end());
        *inout_location = H_quantile(work, 0.5);
    }
    if (out_mad != nullptr) {
        work = abs(work - *inout_location);
        std::sort(work.begin(), work.end());
        *out_mad = 1.4826 * H_quantile(work, 0.5);
    }
}

double H_gaussP(double z)
{
    // sqrt(0.5)
    constexpr double SQRT1_2 = 0.7071067811865475244008443621048490392848;
    return 1.0 - 0.5 * std::erfc(SQRT1_2 * z);
}

double H_gauss(double x)
{
    // 1 / sqrt(2 * pi)
    constexpr double _1_SQRT2PI = 0.3989422804014326779399460599343818684759;
    return _1_SQRT2PI * std::exp(-0.5 * x * x);
}

void H_clip(ArrayXd & x, double min, double max)
{
    for (int i = 0; i < x.size(); ++i) {
        if (x(i) < min)
            x(i) = min;
        else if (x(i) > max)
            x(i) = max;
    }
}


void LPC::Huber::init(huber_s & hs, double windowDuration, int p, double samplingFrequency, double location, bool wantLocation)
{
    const int n = std::round(windowDuration * samplingFrequency);
    hs.e.setZero(n);
    hs.k_stdev = hs.tol = hs.tol_svd = hs.scale = 0.0;
    hs.iter = 1;
    hs.itermax = 1;
    hs.wantLocation = wantLocation;
    if (!wantLocation) hs.location = location;
    hs.wantScale = true;
    hs.n = n;
    hs.p = p;
    hs.w.setZero(n);
    hs.work.setZero(n);
    hs.a.setZero(p);
    hs.c.setZero(p);
    hs.covar.setZero(p, p);
}

void LPC::Huber::getWeights(huber_s & hs, const ArrayXd & e)
{
    assert(e.size() == hs.n);

    const double kstdev = hs.k_stdev * hs.scale;

    for (int i = 0; i < hs.n; ++i) {
        double abs_ei = std::abs(e(i) - hs.location);
        hs.w(i) = abs_ei < kstdev ? 1.0 : kstdev / abs_ei;
    }
}

void LPC::Huber::getWeightedCovars(huber_s & hs, const ArrayXd & s)
{
    assert(s.size() == hs.n);

    const int p = hs.p;

    for (int i = 0; i < p; ++i) {
        for (int j = i; j < p; ++j) {
            double cv1 = 0.0;
            for (int k = p + 1; k < s.size(); ++k) {
                cv1 += s(k - j) * s(k - i) * hs.w(k);
            }
            hs.covar(i, j) = hs.covar(j, i) = cv1;
        }

        double cv2 = 0.0;
        for (int k = p + 1; k < s.size(); ++k) {
            cv2 += s(k - i) * s(k) * hs.w(k);
        }
        hs.c(i) = -cv2;
    }
}

void LPC::Huber::solveLpc(huber_s & hs)
{
    hs.svd = JacobiSVD<MatrixXd>(hs.p, hs.p, ComputeThinU | ComputeThinV);
    hs.svd.setThreshold(hs.tol_svd);
    hs.svd.compute(hs.covar);

    hs.a = hs.svd.solve(hs.c);
}

void LPC::Huber::calc_stat(const ArrayXd &x,
                           double * inout_location, bool wantLocation,
                           double * inout_scale, bool wantScale,
                           double k_stdev, double tol, int maximumNumberOfiterations)
{
    double theta = 2.0 * H_gaussP(k_stdev) - 1.0;
    double beta = theta + k_stdev * k_stdev * (1.0 - theta) - 2.0 * k_stdev * H_gauss(k_stdev);
    double scale;

    H_mad(x, inout_location, wantLocation, &scale);

    if (wantScale) *inout_scale = scale;

    double location0, location1 = *inout_location;
    double scale0, scale1 = *inout_scale;

    int n1 = wantLocation ? x.size() - 1 : x.size(), iter = 0;
    bool locationCriterion, scaleCriterion;
    ArrayXd work(x.size());
    do {
        location0 = location1;
        scale0 = scale1;

        double low = location0 - k_stdev * scale0;
        double high = location0 + k_stdev * scale0;

        work = x;
        H_clip(work, low, high);

        if (wantLocation)
            location1 = work.mean();

        work -= location1;
        double sumsq = (work * work).sum();
        scale1 = std::sqrt(sumsq / (n1 * beta));

        locationCriterion = wantLocation ? std::abs(location0 - location1) > tol * location0 : true;
        scaleCriterion = std::abs(scale0 - scale1) > tol * scale0;
    }
    while (++iter < maximumNumberOfiterations && (scaleCriterion || locationCriterion));

    if (wantLocation) *inout_location = location1;
    if (wantScale ) *inout_scale = scale1;
}