#ifndef ANALYSIS_IRAPT_H
#define ANALYSIS_IRAPT_H

#include <utility>

namespace Analysis {
    struct IRAPT_Cfg {
        float fs;
        float fs_f0_target;
        int src_sub_ratio;
        float fs_f0;
        int FD;
        float max_harmonic_freq;
        int max_harmonic_number;

        float step_sec;
        int step_sub_smp;
        int step_smp;

        float frame_sec;
        int frame_sub_smp;
        int frame_smp;

        float chunk_f0_sec;
        int chunk_f0_size;
        std::vector<float> chunk_f0_freqs;

        std::pair<float, float> f0_limits;
        float f0_max_step;
        std::vector<float> f0_freq_lines;

        struct {
            int FFT_order;
            int FFT_freq_line_size;
            int Interp_factor;
            int Interp_filter_h_size;
            std::vector<float> Interp_filter;
            int Left_index_actual;
            int Right_index_actual;
            int Left_index;
            int Right_index;
            std::vector<int> Actual_indices;
            std::vector<float> Actual_freqs;
            int Actual_freqs_num;
        } corr_param;
    };

    IRAPT_Cfg initCfg(float sampleRate);
}

#endif // ANALYSIS_IRAPT_H
