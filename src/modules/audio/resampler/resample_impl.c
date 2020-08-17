#include "resampler.h"
#include "speex_resampler/resample.c"

EXPORT int speex_resampler_get_required_input_frame_count(SpeexResamplerState* st, uint64_t out_len, uint64_t* in_len)
{
    uint64_t count;

    if (st == NULL || in_len == NULL) {
        return RESAMPLER_ERR_INVALID_ARG;
    }

    *in_len = 0;

    if (out_len == 0) {
        return RESAMPLER_ERR_SUCCESS;   /* Nothing to do. */
    }

    /* we can safely just use channel index 0 for the calculations. */
    if (st->nb_channels == 0) {
        return RESAMPLER_ERR_BAD_STATE;
    }

    count  = out_len * st->int_advance;
    count += (st->samp_frac_num[0] + (out_len * st->frac_advance)) / st->den_rate;

    *in_len = count;

    return RESAMPLER_ERR_SUCCESS;
}

EXPORT int speex_resampler_get_expected_output_frame_count(SpeexResamplerState* st, uint64_t in_len, uint64_t* out_len)
{
    uint64_t count;
    uint64_t last_sample;
    uint32_t samp_frac_num;

    if (st == NULL || out_len == NULL) {
        return RESAMPLER_ERR_INVALID_ARG;
    }

    *out_len = 0;

    if (out_len == 0) {
        return RESAMPLER_ERR_SUCCESS;   /* Nothing to do. */
    }

    /* we can safely just use channel index 0 for the calculations. */
    if (st->nb_channels == 0) {
        return RESAMPLER_ERR_BAD_STATE;
    }

    count         = 0;
    last_sample   = st->last_sample[0];
    samp_frac_num = st->samp_frac_num[0];

    while (!(last_sample >= in_len)) {
        count += 1;

        last_sample   += st->int_advance;
        samp_frac_num += st->frac_advance;
        if (samp_frac_num >= st->den_rate) {
            samp_frac_num -= st->den_rate;
            last_sample += 1;
        }
    }

    *out_len = count;

    return RESAMPLER_ERR_SUCCESS;
}
