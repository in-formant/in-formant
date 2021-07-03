#include "util.h"
#include "aberth.h"
#include <random>

static std::random_device rd;
#if CMAKE_SIZE_OF_VOID_P == 4
static std::mt19937 gen(rd());
#else
static std::mt19937_64 gen(rd());
#endif

static std::pair<double, double> upperLowerBounds(const rpm::vector<double>& P)
{
    const int degree = static_cast<int>(P.size()) - 1;

    double maxUpper = 0.0;
    double maxLower = 0.0;
    for (int i = 0; i < degree + 1; ++i) {
        const double absPi = std::abs(P[i]);
        if (i >= 1 && absPi > maxUpper)
            maxUpper = absPi;
        if (i < degree && absPi > maxLower)
            maxLower = absPi;
    }

    const double upper = 1.0 + 1.0 / std::abs(P[0]) * maxUpper;
    const double lower = std::abs(P[degree]) / (std::abs(P[degree]) + maxLower);

    return { upper, lower };
}

static rpm::vector<std::complex<double>> initRoots(const rpm::vector<double>& P)
{
    const int degree = static_cast<int>(P.size()) - 1;
    const auto [upper, lower] = upperLowerBounds(P);

    static std::uniform_real_distribution<> radius(lower, upper);
    static std::uniform_real_distribution<> angle(0, 2 * M_PI);

    rpm::vector<std::complex<double>> roots;
    for (int i = 0; i < degree; ++i) {
        roots.push_back(std::polar(radius(gen), angle(gen)));
    }
    return roots;
}

static rpm::vector<std::complex<double>> evaluatePolyDer(const rpm::vector<double>& P, const std::complex<double>& x, int numberOfDerivatives)
{
    const int degree = static_cast<int>(P.size()) - 1;
    rpm::vector<std::complex<double>> derivatives(numberOfDerivatives + 1);
    numberOfDerivatives = numberOfDerivatives > degree ? degree : numberOfDerivatives;

    derivatives[0] = P[0];

    for (int i = 1; i <= degree; ++i) {
        const int n = numberOfDerivatives < i ? numberOfDerivatives : i;
        for (int j = n; j >= 1; --j) {
            derivatives[j] = derivatives[j] * x + derivatives[j - 1];
        }
        derivatives[0] = derivatives[0] * x + P[i];
    }

    double fact = 1.0;
    for (int j = 2; j <= numberOfDerivatives; ++j) {
        fact *= j;
        derivatives[j] *= fact;
    }
    return derivatives;
}

static void aberthIterate(const rpm::vector<double>& P, rpm::vector<std::complex<double>>& roots)
{
    int iteration = 0;
    
    while (true) {
        int valid = 0;
        for (int k = 0; k < (int) roots.size(); ++k) {
            auto y = evaluatePolyDer(P, roots[k], 1);
            auto ratio = y[0] / y[1];

            std::complex<double> sum = 0.0;
            for (int j = 0; j < (int) roots.size(); ++j) {
                if (j != k) {
                    sum += 1.0 / (roots[k] - roots[j]);
                }
            }

            auto offset = ratio / (1.0 - ratio * sum);
            if (std::abs(offset.real()) < 1e-10 && std::abs(offset.imag()) < 1e-10) {
                valid++;
            }
            roots[k] -= offset;
        }
        if (valid == (int) roots.size()) {
            break;
        }
        iteration++;
    }
}

rpm::vector<std::complex<double>> Analysis::aberthRoots(
        const rpm::vector<double>& P)
{
    auto roots = initRoots(P);
    
    aberthIterate(P, roots);

    return roots;
}

struct Point {
    double x, y;
    double minDist;
    int cluster;

    inline double distance(const Point& p) const {
        return pow(x - p.x, 2.0) + pow(y - p.y, 2.0);
    }
};

rpm::vector<Point> kMeansClustering(rpm::vector<Point>& points, int epochs, int k)
{
    const int n = points.size();

    rpm::vector<Point> centroids(k);
    rpm::vector<int> nPoints(k);
    rpm::vector<double> sumX(k);
    rpm::vector<double> sumY(k);

    // Pick centroids at random
    for (int i = 0; i < k; ++i) {
        centroids[i] = points[gen() % n];
    }

    // Assign points to a cluster
    for (int clusterId = 0; clusterId < k; ++clusterId) {
        const auto& c = centroids[clusterId];

        for (auto& p : points) {
            const double dist = c.distance(p);
            if (dist < p.minDist) {
                p.minDist = dist;
                p.cluster = clusterId;
            }
        }
    }
    
    for (int iter = 0; iter < epochs; ++iter) {
        std::fill(nPoints.begin(), nPoints.end(), 0);
        std::fill(sumX.begin(), sumX.end(), 0.0);
        std::fill(sumY.begin(), sumY.end(), 0.0);

        // Iterate over points to append data to centroids
        for (auto& p : points) {
            const int clusterId = p.cluster;
            nPoints[clusterId] += 1;
            sumX[clusterId] += p.x;
            sumY[clusterId] += p.y;
            p.minDist = std::numeric_limits<double>::max();
        }

        // Compute the new centroids
        for (int clusterId = 0; clusterId < k; ++clusterId) {
            auto& c = centroids[clusterId];
            c.x = sumX[clusterId] / nPoints[clusterId];
            c.y = sumY[clusterId] / nPoints[clusterId];
        }

        // Assign points to a cluster
        for (int clusterId = 0; clusterId < k; ++clusterId) {
            const auto& c = centroids[clusterId];

            for (auto& p : points) {
                const double dist = c.distance(p);
                if (dist < p.minDist) {
                    p.minDist = dist;
                    p.cluster = clusterId;
                }
            }
        }
    }

    return centroids;
}

rpm::vector<std::complex<double>> Analysis::aberthRootsAroundInitial(
        const rpm::vector<double>& P, double r, double phi, int count)
{
    static std::uniform_real_distribution<> radius(r - 0.15, r + 0.15);
    static std::uniform_real_distribution<> angle(phi - 0.15, phi + 0.15);

    const int degree = static_cast<int>(P.size()) - 1;
    rpm::vector<std::complex<double>> roots;
    for (int i = 0; i < degree; ++i) {
        roots.push_back(std::polar(radius(gen), angle(gen)));
    }
   
    aberthIterate(P, roots);

    rpm::vector<Point> points(degree);
    for (int i = 0; i < degree; ++i) {
        points[i].x = roots[i].real();
        points[i].y = roots[i].imag();
        points[i].minDist = std::numeric_limits<double>::max();
    }

    auto centroids = kMeansClustering(points, 20, count);
    rpm::vector<std::complex<double>> pickedRoots;
    for (const auto& c : centroids) {
        pickedRoots.push_back({c.x, c.y});
    }

    return pickedRoots;
}
