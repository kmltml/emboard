#include "voice_scheduler.h"

#include <stdio.h>

#include "note_source.h"
#include "oscillator.h"

voice_entry voice_table[VOICE_COUNT];
SemaphoreHandle_t voice_table_lock;

void voice_scheduler_init() {
  voice_table_lock = xSemaphoreCreateMutex();
  for(size_t i = 0; i < VOICE_COUNT; i++) {
    voice_table[i].active = 0;
  }
}

void voice_scheduler_task(void* args) {
  note_event ev;
  while(true) {
    BaseType_t recv = xQueueReceive(note_events, &ev, portMAX_DELAY);
    if (recv == pdTRUE) {
      while(xSemaphoreTake(voice_table_lock, portMAX_DELAY) == pdFALSE);

      switch (ev.type) {
      case NE_DOWN:
        voice_table[0].active = true;
        oscillator_reset(&voice_table[0]);
        voice_table[0].note = ev.pitch;
        break;
      case NE_UP:
        voice_table[0].active = false;
        break;
      }

      xSemaphoreGive(voice_table_lock);
    }
  }
}
