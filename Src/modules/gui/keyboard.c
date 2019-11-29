#include "keyboard.h"

#include "../gui.h"
#include "../note_source.h"

#include <stdbool.h>

#include "cmsis_os.h"
#include "stm32746g_discovery_lcd.h"

#define WHITE_KEY_WIDTH 30
#define BLACK_KEY_WIDTH 10
#define WHITE_KEY_HEIGHT 60
#define BLACK_KEY_HEIGHT 40

#define TOP_MARGIN (LCD_HEIGHT - WHITE_KEY_HEIGHT)

static int pressedNotes[TS_MAX_NB_TOUCH];

void initKeyboard() {
    for (size_t i = 0; i < TS_MAX_NB_TOUCH; i++) {
        pressedNotes[i] = -1;
    }
}

void updateKeyboard(TS_StateTypeDef* touchState) {
    static const int white_key_indices[] = {0, 2, 4, 5, 7, 9, 11};
    static const int black_key_indices[] = {1, 3, -1, 6, 8, 10, -1};

    int nextPressedNotes[TS_MAX_NB_TOUCH];

    for (size_t touchIndex = 0; touchIndex < TS_MAX_NB_TOUCH; touchIndex++) {
        int note = -1;
        if (touchIndex < touchState->touchDetected) {
            int x = touchState->touchX[touchIndex];
            int y = touchState->touchY[touchIndex];
            if (TOP_MARGIN <= y && y <= TOP_MARGIN + WHITE_KEY_HEIGHT) {
                if (y <= TOP_MARGIN + WHITE_KEY_HEIGHT / 2) {
                    int key_index =
                        (x - (WHITE_KEY_WIDTH / 2)) / WHITE_KEY_WIDTH;
                    note = black_key_indices[key_index % 7];
                    if (note != -1) {
                        note += 60 + key_index / 7 * 12;
                    }
                }

                if (note ==
                    -1) { // Touch on lower half or there's no black key there
                    int key_index = x / WHITE_KEY_WIDTH;
                    note = white_key_indices[key_index % 7] + 60 +
                           key_index / 7 * 12;
                }
            }
        }
        nextPressedNotes[touchIndex] = note;
    }

    note_event ev;

    // Detect new key presses
    for (size_t i = 0; i < TS_MAX_NB_TOUCH; i++) {
        if (nextPressedNotes[i] != -1) {
            bool found = false;
            for (size_t j = 0; j < TS_MAX_NB_TOUCH; j++) {
                if (pressedNotes[j] == nextPressedNotes[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                ev.type = NE_DOWN;
                ev.pitch = nextPressedNotes[i];
                xQueueSend(note_events, &ev, portMAX_DELAY);
            }
        }
    }
    // Detect key releases
    for (size_t i = 0; i < TS_MAX_NB_TOUCH; i++) {
        if (pressedNotes[i] != -1) {
            bool found = false;
            for (size_t j = 0; j < TS_MAX_NB_TOUCH; j++) {
                if (pressedNotes[i] == nextPressedNotes[j]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                ev.type = NE_UP;
                ev.pitch = pressedNotes[i];
                xQueueSend(note_events, &ev, portMAX_DELAY);
            }
        }
    }
    for (size_t i = 0; i < TS_MAX_NB_TOUCH; i++) {
        pressedNotes[i] = nextPressedNotes[i];
    }
}

void drawKeyboard() {
    const int key_count = 15;
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    for (size_t i = 0; i < key_count; i++) {
        BSP_LCD_DrawRect(i * WHITE_KEY_WIDTH, TOP_MARGIN, WHITE_KEY_WIDTH,
                         WHITE_KEY_HEIGHT);
        if (i % 7 != 2 && i % 7 != 6 && i != key_count - 1) {
            BSP_LCD_FillRect((i + 1) * WHITE_KEY_WIDTH - BLACK_KEY_WIDTH / 2,
                             TOP_MARGIN, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT);
        }
    }
}
