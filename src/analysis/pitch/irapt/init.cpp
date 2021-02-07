#include "irapt.h"
#include <algorithm>
#include <cmath>

using namespace Analysis;

static rpm::vector<double> fir1();

IRAPT_Cfg Analysis::initCfg(double sampleRate)
{
    IRAPT_Cfg cfg;

    cfg.fs = sampleRate;
    cfg.fs_f0_target = 6000;
    cfg.src_sub_ratio = std::round(cfg.fs / cfg.fs_f0_target);
    cfg.fs_f0 = cfg.fs / cfg.src_sub_ratio;
    cfg.FD = 35;
    cfg.max_harmonic_freq = 14000;
    cfg.max_harmonic_number = 100;

    cfg.step_sec = 0.02;
    cfg.step_sub_smp = std::round(cfg.step_sec * cfg.fs_f0);
    cfg.step_smp = cfg.step_sub_smp * cfg.src_sub_ratio;

    cfg.frame_sec = 0.03;
    cfg.frame_sub_smp = std::round(cfg.frame_sec * cfg.fs_f0 / 2) * 2 + 1;
    cfg.frame_smp = std::round(cfg.frame_sec * cfg.fs / 2) * 2 + 1;

    cfg.chunk_f0_sec = 0.3;
    cfg.chunk_f0_size = std::round(cfg.chunk_f0_sec / cfg.step_sec);

    cfg.f0_limits = std::make_pair(50, 450);
    cfg.chunk_f0_freqs.resize(450 - 50 + 1);
    std::iota(cfg.chunk_f0_freqs.begin(), cfg.chunk_f0_freqs.end(), 50);

    cfg.f0_max_step = 23;

    double F = cfg.FD + cfg.FD / 4;
    while (F <= cfg.fs_f0 / 2 - cfg.FD) {
        cfg.f0_freq_lines.push_back(F);
        F += cfg.FD / 2;
    }

    cfg.corr_param
}
