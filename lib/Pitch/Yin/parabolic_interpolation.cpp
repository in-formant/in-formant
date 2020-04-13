//
// Created by rika on 22/11/2019.
//

#include "YIN.h"
#include <iostream>

using namespace Eigen;

double YIN::parabolic_interpolation(Ref<const ArrayXd> array, int tauEstimate) {

    double betterTau;
    int x0, x2;

    const int halfN = array.size();
 
	/* Calculate the first polynomial coeffcient based on the current estimate of tau */
    if (tauEstimate < 1) {
        x0 = tauEstimate;
    }
    else {
        x0 = tauEstimate - 1;
    }

	/* Calculate the second polynomial coeffcient based on the current estimate of tau */
    if (tauEstimate + 1 < halfN) {
		x2 = tauEstimate + 1;
	} 
	else {
		x2 = tauEstimate;
	}
    
    /* Algorithm to parabolically interpolate the shift value tau to find a better estimate */
	if (x0 == tauEstimate) {
		if (array(tauEstimate) <= array(x2)) {
			betterTau = tauEstimate;
		} 
		else {
			betterTau = x2;
		}
	} 
	else if (x2 == tauEstimate) {
		if (array(tauEstimate) <= array(x0)) {
			betterTau = tauEstimate;
		} 
		else {
			betterTau = x0;
		}
	} 
	else {
		float s0, s1, s2;
		s0 = array(x0);
		s1 = array(tauEstimate);
		s2 = array(x2);
		// fixed AUBIO implementation, thanks to Karl Helgason:
		// (2.0f * s1 - s2 - s0) was incorrectly multiplied with -1
		betterTau = tauEstimate + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
	}
    
    return betterTau;
}
