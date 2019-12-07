#include "midi.h"

#include <stdbool.h>
#include <stdint.h>

#include "note_source.h"

#define MIDI_CMD_TYPE_MASK 0xf0

#define MIDI_CMD_NOTE_OFF 0x80
#define MIDI_CMD_NOTE_ON 0x90
#define MIDI_CMD_CTRL_CHANGE 0xB0
#define MIDI_CMD_PITCH_BEND 0xE0

#define MIDI_CMD_MASK 0x80

#define MIDI_DATA_BUFFER_SIZE 16

static uint8_t lastCommand = 0;
static uint8_t dataBuffer[MIDI_DATA_BUFFER_SIZE];
static size_t bufferIndex = 0;

TaskHandle_t midi_task_handle;

QueueHandle_t midi_bytes;

// Return data byte count for the midi command byte.
// Returns 0 if this command is to be ignored.
uint8_t messageLength(uint8_t command) {
    switch (command & MIDI_CMD_TYPE_MASK) {
        case MIDI_CMD_NOTE_OFF:
        case MIDI_CMD_NOTE_ON:
        case MIDI_CMD_CTRL_CHANGE:
        case MIDI_CMD_PITCH_BEND:
            return 2;
        default:
            return 0;
    }
}

void handleMessage(uint8_t command, uint8_t* data) {
    note_event ev;
    switch (command & MIDI_CMD_TYPE_MASK) {
        case MIDI_CMD_NOTE_OFF:
            ev.pitch = data[0];
            ev.type = NE_UP;
            xQueueSend(note_events, &ev, portMAX_DELAY);
            break;
        case MIDI_CMD_NOTE_ON:
            ev.pitch = data[0];
            ev.type = data[1] != 0 ? NE_DOWN : NE_UP;
            xQueueSend(note_events, &ev, portMAX_DELAY);
            break;
    }
}

void receiveByte() {
    SET_BIT(huart6.Instance->CR1, USART_CR1_RXNEIE);
    xQueueReceive(midi_bytes, &dataBuffer[bufferIndex], portMAX_DELAY);
}

void byteReceived() {
    uint8_t byte = dataBuffer[bufferIndex];
    if(byte == 0xfe) {
        return;
    }
    printf("%02x\n", byte);
    if ((byte & MIDI_CMD_MASK) != 0) { // command byte
        bufferIndex = 0;
        lastCommand = (messageLength(byte) == 0) ? 0 : byte;
    } else { // data byte
        if (lastCommand != 0) {
            bufferIndex++;
            if (bufferIndex >= MIDI_DATA_BUFFER_SIZE) {
                // Should never happen
                bufferIndex = 0;
            }
            const uint8_t length = messageLength(lastCommand);
            if (bufferIndex == length) {
                handleMessage(lastCommand, dataBuffer);
                bufferIndex = 0;
            }
        }
    }
}

void midiIRQ() {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(midi_bytes, &huart6.Instance->RDR, &higherPriorityTaskWoken);

    if (higherPriorityTaskWoken) {
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
}

void midi_task(void* args) {
    midi_bytes = xQueueCreate(16, sizeof(uint8_t));
    while (true) {
        receiveByte();
        byteReceived();
    }
}
