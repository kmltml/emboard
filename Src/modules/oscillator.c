#include "oscillator.h"

#include <stdlib.h>

#define SINE_ARG_PREC ((uint32_t) 8)
#define SINE_VAL_PREC ((uint32_t) 8)

const uint16_t sine_cache[257] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
    0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb,
    0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff, 0x100};

static uint16_t base_periods[12];

void oscillator_init() {
    // 2^(1/12) represented as a fraction, for equal temperament calculation
    uint64_t num = 579788;
    uint64_t den = 547247;
    base_periods[0] = 5379;
    for (size_t i = 1; i < 12; i++) {
        base_periods[i] = (uint64_t) base_periods[i - 1] * den / num;
        printf("%d\n", (int32_t) base_periods[i]);
    }

    // Initial values (adjustable via GUI):
    current_settings.osc.shape = 4.1f;
    current_settings.osc.amplitude = 1.0;
    current_settings.osc.tune = 0.0;
}

void oscillator_reset(voice_entry* voice) {
    voice->osc.phase = 0;
}

void oscillator_generate_sine(voice_entry* voice, uint16_t T,
                              uint16_t amplitude) {
    uint16_t phase = voice->osc.phase;

    for (uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
        if (phase >= T)
            phase -= T;

        // Initialize x:
        uint16_t x = phase;

        // If (2 * PI * x / T) is in (PI; 2*PI):
        //  > decrease x to force (2 * PI * x / T) into the range of (0; PI),
        //  > remember to change the sign of the result to negative.
        const bool sign = (x << 1) > T;
        if (sign)
            x -= (T >> 1);

        // If (2 * PI * x / T) is in (PI/2; PI):
        //  > the result is the same as for the argument (2 * PI * (T - 2*x) /
        //  (2 * T)).
        const uint16_t arg =
            ((x << 2) > T)
                ? (((T << (SINE_ARG_PREC + 1)) - (x << (SINE_ARG_PREC + 2))) /
                   T)
                : ((x << (SINE_ARG_PREC + 2)) / T);

        // Fetch cached value:
        int32_t val = sine_cache[arg];

        // Multiply by amplitude and account for scale:
        val = (amplitude * val) >> SINE_VAL_PREC;

        // Account for sign:
        if (sign)
            val = -val;

        // Cast to 16-bit signed integer:
        voice->samples[i] = (int16_t) val;

        phase++;
    }

    voice->osc.phase = phase;
}

void oscillator_generate_square(voice_entry* voice, uint16_t period,
                                uint16_t amplitude) {
    uint16_t phase = voice->osc.phase;

    for (uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
        if (phase >= period)
            phase -= period;

        if (phase < period / 2)
            voice->samples[i] = -((int16_t) amplitude);
        else
            voice->samples[i] = amplitude;

        phase++;
    }

    voice->osc.phase = phase;
}

void oscillator_generate_sawtooth(voice_entry* voice, uint16_t period,
                                  uint16_t amplitude) {
    uint16_t phase = voice->osc.phase;

    for (uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
        if (phase >= period)
            phase -= period;

        voice->samples[i] = 2 * amplitude * phase / period;

        phase++;
    }

    voice->osc.phase = phase;
}

void oscillator_generate_impulse(voice_entry* voice, uint16_t period,
                                 uint16_t amplitude) {
    uint16_t phase = voice->osc.phase;

    for (uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
        if (phase >= period)
            phase -= period;

        if (phase < period / 10)
            voice->samples[i] = amplitude;
        else
            voice->samples[i] = -((int16_t) amplitude);

        phase++;
    }

    voice->osc.phase = phase;
}

void oscillator_generate_triangle(voice_entry* voice, uint16_t period,
                                  uint16_t amplitude) {
    uint16_t phase = voice->osc.phase;

    // HACK. Lazy way to make the oscillator start at 0
    phase += period / 4;

    for (uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
        if (phase >= period)
            phase -= period;

        if (phase <= period / 2) {
            voice->samples[i] =
                phase * 2 * amplitude / (period / 2) - amplitude;
        } else {
            voice->samples[i] =
                (period - phase) * 2 * amplitude / (period / 2) - amplitude;
        }

        phase++;
    }

    phase -= period / 4;

    voice->osc.phase = phase;
}

void oscillator_generate(voice_entry* voice) {
    uint16_t period = base_periods[voice->note % 12];
    period >>= voice->note / 12;

    const float shape = current_settings.osc.shape;
    const uint16_t amplitude =
        (uint16_t) 0x1000 * current_settings.osc.amplitude;

    switch ((uint16_t)(shape + 0.5f)) {
        case 0:
            oscillator_generate_sine(voice, period, amplitude);
            break;
        case 1:
            oscillator_generate_square(voice, period, amplitude);
            break;
        case 2:
            oscillator_generate_sawtooth(voice, period, amplitude);
            break;
        case 3:
            oscillator_generate_impulse(voice, period, amplitude);
            break;
        case 4:
        default:
            oscillator_generate_triangle(voice, period, amplitude);
            break;
    }
}
