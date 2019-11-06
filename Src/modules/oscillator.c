#include "oscillator.h"

#include <stdlib.h>
#include <stdint.h>

static uint16_t base_periods[12];

void oscillator_init() {
  // 2^(1/12) represented as a fraction, for equal temperament calculation
  uint64_t num = 579788;
  uint64_t den = 547247;
  base_periods[0] = 5379; // 8.18 Hz
  for(size_t i = 1; i < 12; i++) {
    base_periods[i] = (uint64_t) base_periods[i - 1] * den / num;
    printf("%d\n", (int32_t) base_periods[i]);
  }
}

void oscillator_reset(voice_entry* voice) {
  voice->osc.phase = 0;
}

void oscillator_generate(voice_entry* voice) {
  uint16_t phase = voice->osc.phase;
  uint16_t period = base_periods[voice->note % 12];
  period >>= voice->note / 12; // Divide by 2 for each octave
  const uint16_t amplitude = 0x5000;
  for(uint16_t i = 0; i < VOICE_BUFFER_SIZE; i++) {
    if(phase >= period) {
      phase -= period;
    }
    if (phase <= period / 2) {
      voice->samples[i] = phase * 2 * amplitude / (period / 2) - amplitude;
    } else {
      voice->samples[i] = (period - phase) * 2 * amplitude / (period / 2) - amplitude;
    }
    phase++;
  }
  voice->osc.phase = phase;
}
