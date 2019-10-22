#include "voice_scheduler.h"

#include <stdio.h>

#include "note_source.h"

voice_entry voice_table[VOICE_COUNT];
SemaphoreHandle_t voice_table_lock;

void voice_scheduler_init() {
  // TODO
}

void voice_scheduler_task(void* args) {
  note_event ev;
  while(true) {
    BaseType_t recv = xQueueReceive(note_events, &ev, portMAX_DELAY);
    if (recv == pdTRUE) {
      printf("[SCHED] %s %d\n", ev.type == NE_DOWN ? "DOWN" : "UP", ev.pitch);
      // TODO insert into voice table
    }
  }
}
