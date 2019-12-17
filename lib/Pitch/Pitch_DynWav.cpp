//
// Created by clo on 13/12/2019.
//

#include <cfloat>
#include <iostream>
#include <vector>
#include "Pitch.h"

using namespace Eigen;

void Pitch::estimate_DynWav(const ArrayXd & _x, double fs, Pitch::Estimation & result,
                            int maxLevels, double maxF, int differenceLevels, double maximaThresholdRatio, double oldFreq)
{
    // Resize to a multiple of 64.
    int numSamples = (_x.size() / 64) * 64;
    ArrayXd x = _x.head(numSamples);

    ArrayXd a, d, aprev;

    std::vector<int> maxIndices;
    std::vector<int> minIndices;
    std::vector<int> differs;

    double oldMode;
    int prevMaxCount;
    int prevMinCount;

    double aver = 0.0;

    double globalMax = -DBL_MAX;
    double globalMin = DBL_MAX;
    for (int i = 0; i < numSamples; ++i) {
        double sample = x(i);
        aver += sample;
        globalMax = std::max(globalMax, sample);
        globalMin = std::min(globalMin, sample);
    }
    aver /= static_cast<double>(numSamples);

    double maxThresh = maximaThresholdRatio * (globalMax - aver) + aver;
    double minThresh = maximaThresholdRatio * (globalMin - aver) + aver;

    oldMode = oldFreq > 0 ? fs / oldFreq : 0;
    prevMaxCount = 0;
    prevMinCount = 0;

    aprev = x;

    for (int i = 2; i <= maxLevels; ++i) {
        int newWidth = numSamples / pow(2, i - 1);

        // Perform the FLWT.

        a.resize(newWidth);
        d.resize(newWidth);
        for (int j = 0; j < newWidth; ++j) {
            d(j) = aprev(2 * j + 1) - aprev(2 * j);
            a(j) = aprev(2 * j) + d(j) / 2.0;
        }

        // Find the maxes of the current approximation.

        int minDist = std::max<int>(fs / maxF / pow(2, i - 1), 1);

        int climber = 0; // 1 if pos, -1 if neg.
        if (a(1) > a(0))
            climber = 1;
        else
            climber = -1;

        int canExt = 1;   // Tracks whether an extreme can be found (based on zero crossings).
        int tooClose = 0; // Tracks how many more samples must be moved before another extreme.

        maxIndices.clear();
        minIndices.clear();

        for (int j = 1; j < newWidth - 1; ++j) {
            double test = a(j) - a(j - 1);

            if (climber >= 0 && test < 0) {
                if (a(j - 1) >= maxThresh && canExt && !tooClose) {
                    maxIndices.push_back(j - 1);
                    canExt = 0;
                    tooClose = minDist;
                }
                climber = -1;
            }
            else if (climber <= 0 && test > 0) {
                if (a(j - 1) <= minThresh && canExt && !tooClose) {
                    minIndices.push_back(j - 1);
                    canExt = 0;
                    tooClose = minDist;
                }
                climber = 1;
            }

            if ((a(j) <= aver && a(j - 1) > aver) || (a(j) >= aver && a(j - 1) < aver)) {
                canExt = 1;    
            }

            if (tooClose) {
                tooClose--;
            }
        }

        // Calculate the mode distance between peaks at each level.

        const int maxCount = maxIndices.size();
        const int minCount = minIndices.size();

        if (maxCount >= 2 && minCount >= 2) {

            // Calculate the differences at differenceLevels distances.

            differs.clear();
            for (int j = 1; j <= differenceLevels; ++j) { // Interval of differences (neighbour, next-neighbour)
                for (int k = 0; k < maxCount - j; ++k) {
                    differs.push_back(abs(maxIndices[k + j] - maxIndices[k]));
                }
                for (int k = 0; k < minCount - j; ++k) {
                    differs.push_back(abs(minIndices[k + j] - minIndices[k]));
                }
            }

            const int dCount = differs.size();

            // Find the center mode of the differences.

            double numer = 1; // Require at least two agreeing differs to yield a mode.
            double mode = 0;  // If none is found, leave as zero.

            for (int j = 0; j < dCount; ++j) {
                
                // Find the # of times that distance j is within minDist samples of another distance.
                int numerJ = 0;

                for (int k = 0; k < dCount; ++k) {
                    if (abs(differs[k] - differs[j]) <= minDist)
                        numerJ++;
                }

                // If there are more, set the new standard.
                if (numerJ >= numer && numerJ > newWidth / differs[j] / 4) {
                    if (numerJ == numer) {
                        if (oldMode && abs(differs[j] - oldMode / pow(2, i - 1)) < minDist) {
                            mode = differs[j];
                        }
                        else if (!oldMode && differs[j] > 1.95 * mode && differs[j] < 2.05 * mode) {
                            mode = differs[j];
                        }
                    }
                    else {
                        numer = numerJ;
                        mode = differs[j];
                    }
                }
                else if (numerJ == numer - 1 && oldMode && abs(differs[j] - oldMode / pow(2, i - 1)) < minDist) {
                    mode = differs[j];
                }
            }

            // Set the mode via averaging.

            if (mode) {
                double avg = 0.0;
                double num = 0;

                for (int k = 0; k < dCount; ++k) {
                    if (abs(mode - differs[k]) <= minDist) {
                        avg += differs[k];
                        num++;
                    }
                }
                mode = avg / num;
            }

            // Determine if the modes are shared.

            if (oldMode && prevMaxCount >= 2 && prevMinCount >= 2) {
                
                // If the modes are within a sample of one another, return the calculated frequency.
                if (abs(oldMode - 2 * mode) <= minDist) {
                    result.pitch = fs / oldMode / pow(2, i - 2);
                    result.isVoiced = true;
                    return;
                }
            }
 
            oldMode = mode;
            prevMinCount = minCount;
            prevMaxCount = maxCount;
        }
    }

    result.pitch = 0;
    result.isVoiced = false;
}
