#include "keyboard.h"

#include "../note_source.h"
#include "../gui.h"

#include <stdbool.h>

#include "cmsis_os.h"
#include "stm32746g_discovery_lcd.h"

#define WHITE_KEY_WIDTH 30
#define BLACK_KEY_WIDTH 10
#define WHITE_KEY_HEIGHT 60
#define BLACK_KEY_HEIGHT 40

#define TOP_MARGIN (LCD_HEIGHT - WHITE_KEY_HEIGHT)

static int pressedNote = -1;

void updateKeyboard(TS_StateTypeDef* touchState) {
    static const int white_key_indices[] = {0, 2, 4, 5, 7, 9, 11};
    static const int black_key_indices[] = {1, 3, -1, 6, 8, 10, -1};

    int note = -1;
    if (touchState->touchDetected > 0) {
        int x = touchState->touchX[0];
        int y = touchState->touchY[0];
        if (TOP_MARGIN <= y && y <= TOP_MARGIN + WHITE_KEY_HEIGHT) {
            if (y <= TOP_MARGIN + WHITE_KEY_HEIGHT / 2) {
                int key_index = (x - (WHITE_KEY_WIDTH / 2)) / WHITE_KEY_WIDTH;
                note = black_key_indices[key_index % 7];
                if (note != -1) {
                    note += 60 + key_index / 7 * 12;
                }
            }

            if (note == -1) { // Touch on lower half or there's no black key there
                int key_index = x / WHITE_KEY_WIDTH;
                note = white_key_indices[key_index % 7] + 60 + key_index / 7 * 12;
            }
        }
    }
    if (pressedNote != note) {
        note_event ev = {pressedNote, NE_UP};
        xQueueSend(note_events, &ev, portMAX_DELAY);
        ev.pitch = note;
        ev.type = NE_DOWN;
        xQueueSend(note_events, &ev, portMAX_DELAY);
        pressedNote = note;
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
