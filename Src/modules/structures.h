#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float shape;
  float amplitude;
  float tune;
} oscillator_settings;

typedef struct {
  uint16_t phase;
} oscillator_state;

// Main audio out buffer size in samples
#define AUDIO_OUT_BUFFER_SIZE 4096

// TODO determine voice buffer size
#define VOICE_BUFFER_SIZE (AUDIO_OUT_BUFFER_SIZE / 2)

typedef struct {
  bool active;
  uint8_t note;
  oscillator_state osc;
  int16_t samples[VOICE_BUFFER_SIZE];
} voice_entry;
