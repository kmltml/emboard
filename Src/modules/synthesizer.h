#pragma once
#include "voice_scheduler.h"
#include "oscillator.h"

typedef struct {
  oscillator_settings osc;
} settings;

extern settings current_settings;

void synthesizer_init();

void synthesize(voice_entry* voice);

void mix();

void synthesizer_task(void* args);
