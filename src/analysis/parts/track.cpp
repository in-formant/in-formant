#include "../Analyser.h"

void Analyser::trackFormants() {
   
    Formant::Frames finalTrack(frameCount);

    Formant::Frames voicedSegment;
    int start;

    int i = 0;

    while (i < frameCount) {

        // This frame is voiced.
        if (pitchTrack[i] != 0) {

            start = i;

            voicedSegment.push_back(formantTrack[i]);

            while (i + 1 < frameCount && pitchTrack[i + 1] != 0) {
                voicedSegment.push_back(formantTrack[i + 1]);
                i++;
            }

            if (i == start) {
                finalTrack[i] = formantTrack[i];
                i++;
            }
            else {
                bool result = Formant::track(
                    voicedSegment,
                    3,
                    550,
                    1650,
                    2750,
                    3850,
                    4950,
                    1.0,
                    0.2,
                    0.6
                );

                for (int j = 0; j < voicedSegment.size(); ++j) {
                    finalTrack[start + j] = voicedSegment[j];
                }
            }

            voicedSegment.clear();
        }
        else {
            finalTrack[i] = formantTrack[i];
            i++;
        }

    }

    formantTrack = std::move(finalTrack);

}
