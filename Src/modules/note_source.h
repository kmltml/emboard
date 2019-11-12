#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

typedef enum { NE_DOWN, NE_UP } note_event_type;

typedef struct {
    uint8_t pitch;
    note_event_type type;
} note_event;

extern QueueHandle_t note_events;

void note_source_init();

void note_source_task(void* args);
