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
    voice_table[i].osc.damping = 0;
  }
}

void voice_scheduler_task(void* args) {
  note_event ev;
  while(true) {
    BaseType_t recv = xQueueReceive(note_events, &ev, portMAX_DELAY);
    if (recv == pdTRUE) {
      while(xSemaphoreTake(voice_table_lock, portMAX_DELAY) == pdFALSE);

      if(ev.type == NE_DOWN) {
        size_t first_inactive_voice = VOICE_COUNT;
        size_t least_loud_damping_voice = VOICE_COUNT;
        uint16_t least_loud_damping_amplitude = (uint16_t)(-1);
        for(size_t i = 0; i < VOICE_COUNT; ++i) {
          if(voice_table[i].active) {
            if(voice_table[i].osc.damping && voice_table[i].osc.amplitude < least_loud_damping_amplitude) {
              least_loud_damping_voice = i;
              least_loud_damping_amplitude = voice_table[i].osc.amplitude;
            }
          }
          else {
            first_inactive_voice = i;
            break;
          }
        }

        size_t selected_voice = first_inactive_voice;
        if(selected_voice == VOICE_COUNT) selected_voice = least_loud_damping_voice;

        if(selected_voice < VOICE_COUNT) {
          voice_table[selected_voice].active = true;
          oscillator_reset(&voice_table[selected_voice]);
          voice_table[0].note = ev.pitch;
        }
        //Otherwise, the input is discarded.
      }
      else if(ev.type == NE_UP) {
        for(size_t i = 0; i < VOICE_COUNT; ++i) {
          if(voice_table[i].active && voice_table[i].note == ev.pitch) {
            voice_table[i].osc.damping = true;
            break;
          }
        }
      }

      xSemaphoreGive(voice_table_lock);
    }
  }
}
