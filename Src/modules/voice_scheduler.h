#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#define VOICE_COUNT 8
// TODO determine voice buffer size
#define VOICE_BUFFER_SIZE 1024

typedef struct {
  bool active;
  uint8_t note;
  uint16_t samples[VOICE_BUFFER_SIZE];
} voice_entry;

voice_entry voice_table[VOICE_COUNT];

SemaphoreHandle_t voice_table_lock;

void voice_scheduler_init();

void voice_scheduler_task(void* args);
