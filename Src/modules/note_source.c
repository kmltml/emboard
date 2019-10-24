#include "note_source.h"
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"

QueueHandle_t note_events;

void note_source_init() {
  note_events = xQueueCreate(8, sizeof(note_event));
}

void note_source_task(void* args) {
  note_event ev = { 60, NE_DOWN };
  while(true) {
    ev.type = NE_DOWN;
    xQueueSend(note_events, &ev, portMAX_DELAY);
    osDelay(750);
    ev.type = NE_UP;
    xQueueSend(note_events, &ev, portMAX_DELAY);
    osDelay(250);
  }
}
