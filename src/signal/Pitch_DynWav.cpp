//
// Created by clo on 13/09/2019.
//

#include "Pitch.h"

using namespace Eigen;

static constexpr int maxFLWTlevels = 6;
static constexpr double maxFrequency = 3000;
static constexpr int differenceLevels = 3;
static constexpr double amplitudeThresholdRatio = 0.75;

void Pitch::estimate_DynWav(const ArrayXd &x, double fs, Pitch::Estimation & result) {

    ArrayXd buffer = x;

    int currentSampleCount = buffer.size();

    std::vector<int> mins, maxs;
    ArrayXi distances;

    mins.reserve(currentSampleCount);
    maxs.reserve(currentSampleCount);
    distances.resize(currentSampleCount);

    // Estimate then subtract the DC component.
    double theDC = buffer.mean();
    buffer -= theDC;

    double amplitudeThreshold = buffer.abs().maxCoeff() * amplitudeThresholdRatio;

    int currentLevel = 0;
    double currentModeDistance = -1;
    int delta;

    result.pitch = -1;

    while (true) {

        delta = static_cast<int>(fs / pow(2, currentLevel) * maxFrequency);

        if (currentSampleCount < 2) {
            goto end;
        }

        // Compute the first maxima and minima after zero-crossing
        // Store if greater than the min threshold
        // and if at greater distance than delta.
        double dv, previousDV(-HUGE_VAL);

        mins.clear();
        maxs.clear();

        int lastMinIndex(INT_MIN);
        int lastMaxIndex(INT_MIN);
        bool findMin(false);
        bool findMax(false);

        for (int i = 2; i < currentSampleCount; ++i) {
            double si = buffer[i];
            double sim1 = buffer[i - 1];

            if (sim1 <= 0 && si > 0) findMax = true;
            if (sim1 >= 0 && si < 0) findMin = true;

            dv = si - sim1;

            if (previousDV > -HUGE_VAL) {
                if (findMin && previousDV < 0 && dv >= 0) {
                    if (abs(si) >= amplitudeThreshold && i > lastMinIndex + delta) {
                        mins.push_back(i);
                        lastMinIndex = i;
                        findMin = false;
                    }
                }

                if (findMax && previousDV > 0 && dv <= 0) {
                    if (abs(si) >= amplitudeThreshold && i > lastMaxIndex + delta) {
                        maxs.push_back(i);
                        lastMaxIndex = i;
                        findMax = false;
                    }
                }
            }

            previousDV = dv;
        }

        if (mins.empty() && maxs.empty()) {
            goto end;
        }

        int d;
        int nbMins = mins.size();
        distances.setZero();

        for (int i = 0; i < nbMins; ++i) {
            for (int j = 1; j < differenceLevels; ++j) {
                if (i + j < nbMins) {
                    d = abs(mins[i] - mins[i + j]);

                    distances(d) += 1;
                }
            }
        }

        int bestDistance = -1;
        int bestValue = -1;

        for (int i = 0; i < currentSampleCount; ++i) {
            int summed = 0;
            for (int j = -delta; j <= delta; ++j) {
                if (i + j >= 0 && i + j < currentSampleCount) {
                    summed += distances(i + j);
                }
            }

            if (summed == bestValue && (i == 2 * bestDistance)) {
                bestDistance = i;
            }
            else if (summed > bestValue) {
                bestValue = summed;
                bestDistance = i;
            }
        }

        // averaging
        double distAvg = 0;
        double nbDists = 0;
        for (int j = -delta; j <= delta; ++j) {
            if (bestDistance + j >= 0 && bestDistance + j < currentSampleCount) {
                int nbDist = distances(bestDistance + j);
                if (nbDist > 0) {
                    nbDists += nbDist;
                    distAvg += (bestDistance + j) * nbDist;
                }
            }
        }

        // this is our mode distance!
        distAvg /= nbDists;

        // continue the levels?
        if (currentModeDistance > -1) {
            double similarity = abs(distAvg * 2 - currentModeDistance);
            if (similarity <= 2 * delta) {
                // two consecutive similar mode distances : ok !
                result.pitch = static_cast<double>(fs) / pow(2.0, currentLevel - 1) * currentModeDistance;
                goto end;
            }
        }

        // not similar, continue next level
        currentModeDistance = distAvg;

        currentLevel++;
        if (currentLevel >= maxFLWTlevels) {
            goto end;
        }

        // downsample
        if (currentSampleCount < 2) {
            goto end;
        }

        for (int i = 0; i < currentSampleCount / 2; ++i) {
            buffer(i) = (buffer(2 * i) + buffer(2 * i + 1)) / 2.0;
        }

        currentSampleCount /= 2;
        buffer.resize(currentSampleCount);
    }

end:

    result.probability = -1;
    result.isVoiced = (result.pitch != -1);

    if (!result.isVoiced) {
        result.pitch = NAN;
    }

}