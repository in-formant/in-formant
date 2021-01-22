#include "pitch.h"
#include <algorithm>
#include <complex>
#include <float.h>
#include <map>
#include <numeric>
#include <vector>
#include "../util/util.h"

#define MPM_CUTOFF 0.93
#define MPM_SMALL_CUTOFF 0.5
#define MPM_LOWER_PITCH_CUTOFF 80.0
#define PMPM_PA 0.01
#define PMPM_N_CUTOFFS 20
#define PMPM_PROB_DIST 0.05
#define PMPM_CUTOFF_BEGIN 0.8
#define PMPM_CUTOFF_STEP 0.01

template <typename T>
static std::vector<int>
peak_picking(const std::vector<T> &nsdf)
{
	std::vector<int> max_positions{};
	int pos = 0;
	int cur_max_pos = 0;
	ssize_t size = nsdf.size();

	while (pos < (size - 1) / 3 && nsdf[pos] > 0)
		pos++;
	while (pos < size - 1 && nsdf[pos] <= 0.0)
		pos++;

	if (pos == 0)
		pos = 1;

	while (pos < size - 1) {
		if (nsdf[pos] > nsdf[pos - 1] && nsdf[pos] >= nsdf[pos + 1] &&
		    (cur_max_pos == 0 || nsdf[pos] > nsdf[cur_max_pos])) {
			cur_max_pos = pos;
		}
		pos++;
		if (pos < size - 1 && nsdf[pos] <= 0) {
			if (cur_max_pos > 0) {
				max_positions.push_back(cur_max_pos);
				cur_max_pos = 0;
			}
			while (pos < size - 1 && nsdf[pos] <= 0.0) {
				pos++;
			}
		}
	}
	if (cur_max_pos > 0) {
		max_positions.push_back(cur_max_pos);
	}
	return max_positions;
}

inline int
pow2roundup (int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

template <typename T>
void
acorr_r(std::vector<T> &audio_buffer)
{
	if (audio_buffer.size() == 0)
		throw std::invalid_argument("audio_buffer shouldn't be empty");

        const int N = audio_buffer.size();

        int nfft = 2 * N - 1;
 
        static std::shared_ptr<Analysis::RealFFT> fft(nullptr);

        if (!fft || fft->getInputLength() != nfft) {
            fft.reset(new Analysis::RealFFT(nfft));
        }

        for (int i = 0; i < N; ++i) {
            fft->input(i) = audio_buffer[i];
        }
        for (int i = N; i < nfft; ++i) {
            fft->input(i) = 0.0;
        }

        fft->computeForward();
        
        for (int i = 0; i < nfft / 2 + 1; ++i) {
            std::dcomplex z = fft->output(i);
            fft->output(i) = (z * conj(z)) / (double) nfft;
        }
        fft->computeBackward();

        for (int i = 0; i < N; ++i) {
            audio_buffer[i] = fft->input(i);
        }
}

Analysis::PitchResult
Analysis::Pitch::MPM::solve(const double *data, int length, int sample_rate)
{
    using T = double;

    std::vector<T> audio_buffer(data, data + length);

	acorr_r(audio_buffer);

        double max = 0.02;
        for (int i = 0; i < length; ++i) {
            if (fabs(audio_buffer[i]) > max) {
                max = fabs(audio_buffer[i]);
            }
        }

        for (int i = 0; i < length; ++i) {
            audio_buffer[i] /= max;
        }

	std::vector<int> max_positions = peak_picking(audio_buffer);
	std::vector<std::pair<T, T>> estimates;

	T highest_amplitude = -DBL_MAX;

	for (int i : max_positions) {
		highest_amplitude = std::max(highest_amplitude,audio_buffer[i]);
		if (audio_buffer[i] > MPM_SMALL_CUTOFF) {
			auto x = parabolicInterpolation(audio_buffer, i);
			estimates.push_back(x);
			highest_amplitude = std::max(highest_amplitude, std::get<1>(x));
		}
	}

	if (estimates.empty())
		return { .pitch = 0.0, .voiced = false };

	T actual_cutoff = MPM_CUTOFF * highest_amplitude;
	T period = 0;

	for (auto i : estimates) {
		if (std::get<1>(i) >= actual_cutoff) {
			period = std::get<0>(i);
			break;
		}
	}

	T pitch_estimate = ((double) sample_rate / period);

	return (pitch_estimate > MPM_LOWER_PITCH_CUTOFF)
                    ? Analysis::PitchResult { .pitch = pitch_estimate, .voiced = true }
                    : Analysis::PitchResult { .pitch = 0.0, .voiced = false };
}
