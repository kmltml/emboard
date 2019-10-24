#pragma once

#include "FreeRTOS.h"
#include "task.h"

#include "voice_scheduler.h"
#include "oscillator.h"

typedef struct {
  oscillator_settings osc;
} settings;

extern settings current_settings;

void synthesizer_init();

void synthesize(voice_entry* voice);

void mix();

extern TaskHandle_t synthesizer_task_handle;

void synthesizer_task(void* args);
