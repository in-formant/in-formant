#include "../Analyser.h"

void Analyser::trackFormants() {
   
    Formant::Frames finalTrack(frameCount);

    Formant::Frames voicedSegment;
    int start;

    int i = 0;

    while (i < frameCount) {

        // This frame is voiced.
        if (pitchTrack[i] != 0 && formantTrack[i].nFormants >= 3) {

            start = i;

            voicedSegment.clear();
            voicedSegment.push_back(formantTrack[i]);

            while (i + 1 < frameCount && pitchTrack[i + 1] != 0 && formantTrack[i + 1].nFormants >= 3) {
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
                    1.0,
                    2.0
                );

                for (int j = 0; j < voicedSegment.size(); ++j) {
                    finalTrack[start + j] = voicedSegment[j];
                }
            }
        }
        else {
            finalTrack[i] = formantTrack[i];
            i++;
        }

    }

    formantTrack = std::move(finalTrack);

}
