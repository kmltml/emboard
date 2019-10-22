#pragma once

#include "voice_scheduler.h"

typedef struct {
  float shape;
  float amplitude;
  float tune;
} oscillator_settings;

void oscillator_generate(voice_entry* voice);
