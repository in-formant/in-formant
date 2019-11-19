//
// Created by clo on 19/11/2019.
//

#include "../Analyser.h"
#include "../../lib/Pitch/YAAPT/YAAPT.h"
#include "../../lib/FFT/FFT.h"

void Analyser::refinePitch() {
    YAAPT::Result yaapt;
    YAAPT::Params params;
    YAAPT::getF0_slow(audioFrames, fs, yaapt, params);

    YAAPT::ptch_fix(yaapt.pitch, 1, 2);

    // replace the last pitch tracks with the refined track.
    int len = yaapt.pitch.size();
    for (int i = 0; i < len; ++i) {
        int j = analysisFrameCount - len + i;
        if (pitchTrack[j] > 0) {
            pitchTrack[j] = yaapt.pitch(i);
        }
    }

    refineCount++;
    if (refineCount % analysisCleanupFftTime == 0) {
        all_fft_cleanup();
        refineCount = 0;
    }
}