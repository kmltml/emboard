#include "oscillator.h"

#include <stdlib.h>
#include <stdint.h>

static uint16_t base_periods[12];

void oscillator_init() {
  // 2^(1/12) represented as a fraction, for equal temperament calculation
  uint64_t num = 579788;
  uint64_t den = 547247;
  base_periods[0] = 5379;
  for(size_t i = 1; i < 12; i++) {
    base_periods[i] = (uint64_t) base_periods[i - 1] * den / num;
    printf("%d\n", (int32_t) base_periods[i]);
  }
}

void oscillator_reset(voice_entry* voice) {
  voice->osc.phase = 75;
}

void oscillator_generate_sine(voice_entry* voice, uint16_t period, uint16_t amplitude) {
	uint16_t phase = voice->osc.phase;

	for(uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
		if(phase >= period)
			phase -= period;

		//TODO: sine implementation using pre-calculated buffer
		voice->samples[i] = 0;

		phase++;
	}

	voice->osc.phase = phase;
}

void oscillator_generate_square(voice_entry* voice, uint16_t period, uint16_t amplitude) {
	uint16_t phase = voice->osc.phase;

	for(uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
		if(phase >= period)
			phase -= period;

		if(phase < period / 2)
			voice->samples[i] = -((int16_t)amplitude);
		else
			voice->samples[i] = amplitude;

		phase++;
	}

	voice->osc.phase = phase;
}


void oscillator_generate_sawtooth(voice_entry* voice, uint16_t period, uint16_t amplitude) {
	uint16_t phase = voice->osc.phase;

	for(uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
		if(phase >= period)
			phase -= period;

		voice->samples[i] = 2 * amplitude * phase / period;

		phase++;
	}

	voice->osc.phase = phase;
}

void oscillator_generate_impulse(voice_entry* voice, uint16_t period, uint16_t amplitude) {
	uint16_t phase = voice->osc.phase;

	for(uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
		if(phase >= period)
			phase -= period;

		if(phase < period / 10)
			voice->samples[i] = amplitude;
		else
			voice->samples[i] = -((int16_t)amplitude);

		phase++;
	}

	voice->osc.phase = phase;
}

void oscillator_generate_triangle(voice_entry* voice, uint16_t period, uint16_t amplitude) {
	uint16_t phase = voice->osc.phase;

	for(uint16_t i = 0; i < VOICE_BUFFER_SIZE; ++i) {
		if(phase >= period)
			phase -= period;

		if (phase <= period / 2) {
		  voice->samples[i] = phase * 2 * amplitude / (period / 2) - amplitude;
		} else {
		  voice->samples[i] = (period - phase) * 2 * amplitude / (period / 2) - amplitude;
		}

		phase++;
	}

	voice->osc.phase = phase;
}

void oscillator_generate(voice_entry* voice) {
  uint16_t period = base_periods[voice->note % 12];
  period >>= voice->note / 12;
  const uint16_t amplitude = 0x5000;

  float xshape = current_settings.osc.shape;

  float shape = 4.0f;

  switch((uint16_t)(shape + 0.5f)) {
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
