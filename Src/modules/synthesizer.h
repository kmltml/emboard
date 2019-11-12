#pragma once

#include "FreeRTOS.h"
#include "task.h"

#include "oscillator.h"
#include "voice_scheduler.h"

void synthesizer_init();

void synthesize(voice_entry* voice);

void mix();

extern TaskHandle_t synthesizer_task_handle;

void synthesizer_task(void* args);
