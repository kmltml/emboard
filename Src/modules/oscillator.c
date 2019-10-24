#include "oscillator.h"

void oscillator_reset(voice_entry* voice) {
  voice->osc.phase = 75;
}

void oscillator_generate(voice_entry* voice) {
  uint16_t phase = voice->osc.phase;
  const uint16_t period = 300;
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
