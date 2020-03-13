//
// Created by clo on 13/12/2019.
//

#include <cfloat>
#include <iostream>
#include <vector>
#include <map>
#include "Pitch.h"

using namespace Eigen;

void Pitch::estimate_DynWav(const ArrayXd & _x, double fs, Pitch::Estimation & result,
                            int lev, double maxFreq, int diffLevs, double globalMaxThresh, double oldFreq)
{
    // Resize to a multiple of 64.
    int dataLen = (_x.size() / 64) * 64;

    // Set old mode if old freq is set.
    double oldMode = oldFreq > 0 ? fs / oldFreq : 0;
    
    std::vector<int> maxCount(lev, 0);
    std::vector<int> minCount(lev, 0);
    std::vector<int> mode(lev, 0);
    
    std::vector<int> maxIndices;
    std::vector<int> minIndices;

    std::vector<ArrayXd> a(lev);
    a[0] = _x.head(dataLen);

    double aver = a[0].mean();
    double globalMax = a[0].maxCoeff();
    double globalMin = a[0].minCoeff();
    double maxThresh = globalMaxThresh * (globalMax - aver) + aver; // Adjust for DC offset
    double minThresh = globalMaxThresh * (globalMin - aver) + aver;

    // Begin pitch detection
    
    for (int i = 1; i < lev; ++i) {
        int newWidth = dataLen / (2 << (i - 1));

        // Perform the FLWT.
        
        ArrayXd d(newWidth);
        ArrayXd acur(newWidth);
        for (int j = 0; j < newWidth; ++j) {
            d(j) = a[i - 1](2 * j + 1) - a[i - 1](2 * j);
            acur(j) = a[i - 1](2 * j) + d(j) / 2;
        }
        a[i] = std::move(acur);

        // Find the maxes of the current approximation

        int minDist = std::max<int>(floor(fs / maxFreq / (2 << (i - 1))), 1);
        maxCount[i] = 0;
        minCount[i] = 0;
        maxIndices.clear();
        minIndices.clear();

        int climber = (a[i](1) - a[i](0) > 0) ? 1 : -1;

        bool canExt = true; // Tracks whether an extreme can be found (based on zero-crossings)
        int tooClose = 0;   // Tracks how many more samples must be moved before another extreme

        for (int j = 1; j < newWidth - 1; ++j) {
            double test = a[i](j) - a[i](j - 1);

            if (climber >= 0 && test < 0) {
                if (a[i](j - 1) >= maxThresh && canExt && tooClose == 0) {
                    maxCount[i]++;
                    maxIndices.push_back(j - 1);
                    canExt = false;
                    tooClose = minDist;
                }
                climber = -1;
            }
            else if (climber <= 0 && test > 0) {
                if (a[i](j - 1) <= minThresh && canExt && tooClose == 0) {
                    minCount[i]++;
                    minIndices.push_back(j - 1);
                    canExt = false;
                    tooClose = minDist;
                }
                climber = 1;
            }

            if ((a[i](j) <= aver && a[i](j - 1) > aver) || (a[i](j) >= aver && a[i](j - 1) < aver)) {
                canExt = true;
            }

            if (tooClose != 0) {
                tooClose--;
            }
        }

        // Calculate the mode distance between peaks at each level
        
        if (maxCount[i] >= 2 && minCount[i] >= 2) {

            // Calculate the differences at diffLevs distances
            
            std::vector<int> differs;
            for (int j = 1; j <= diffLevs; ++j) { // Interval of differences (neighbor, next-neighbor)
                for (int k = 0; k < maxCount[i] - j; ++k) { // Starting point of each run
                    differs.push_back(maxIndices[k + j] - maxIndices[k]);
                }
                for (int k = 0; k < minCount[i] - j; ++k) { // Starting point of each run
                    differs.push_back(minIndices[k + j] - minIndices[k]);
                }
            }
            
            int dCount = differs.size();

            // Find the center mode of the differences

            int numer = 1; // Require at least two agreeing differs to yield a mode
            mode[i] = 0;   // If none is found, leave as zero

            for (int j = 0; j < dCount; ++j) {
                
                // Find the # of times that distance j is within minDist samples of another distance
                int numerJ = 0;
                for (int k = 0; k < dCount; ++k) {
                    if (std::abs(differs[k] - differs[j]) <= minDist)
                        numerJ++;
                }

                // If there are more, set the new standard
                if (numerJ >= numer && numerJ > floor(newWidth / differs[j]) / 4) {
                    if (numerJ == numer) {
                        if (oldMode != 0 && std::abs(differs[j] - oldMode / (2 << (i - 1))) < minDist) {
                            mode[i] = differs[j];
                        }
                        else if (oldMode == 0 && (differs[j] > 1.95 * mode[i] && differs[j] < 2.05 * mode[i])) {
                            mode[i] = differs[i];
                        }
                    }
                    else {
                        numer = numerJ;
                        mode[i] = differs[j];
                    }
                }
                else if (numerJ == numer - 1 && oldMode != 0 && std::abs(differs[j] - oldMode / (2 << (i - 1))) < minDist) {
                    mode[i] = differs[j];
                }
            }

            // Set the mode via averaging.
            
            if (mode[i] != 0) {
                double mean = 0;
                double meanCount = 0;
                for (int k = 0; k < dCount; ++k) {
                    if (std::abs(mode[i] - differs[k]) <= minDist) {
                        mean += differs[k];
                        meanCount++;
                    }
                }
                mode[i] = mean / meanCount;
            }

            // Determine if the modes are shared
            
            if (mode[i - 1] != 0 && maxCount[i - 1] >= 2 && minCount[i - 1] >= 2) {

                // If the modes are within a sample of one another, return the calculated frequency
                if (std::abs(mode[i - 1] - 2 * mode[i]) <= minDist) {
                    result.isVoiced = true;
                    result.pitch = fs / mode[i - 1] / (2 << (i - 2));
                    return;
                }
            }
        }
    }

    result.isVoiced = false;
}
