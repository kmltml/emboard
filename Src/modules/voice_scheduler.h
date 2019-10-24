#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "structures.h"

#define VOICE_COUNT 8

extern voice_entry voice_table[VOICE_COUNT];

extern SemaphoreHandle_t voice_table_lock;

void voice_scheduler_init();

void voice_scheduler_task(void* args);
