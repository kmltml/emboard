#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float shape;
    float amplitude;
    float tune;
    float velocity_response;
} oscillator_settings;

typedef struct {
    uint32_t phase;
} oscillator_state;

typedef enum {
    ENVELOPE_ATTACK,
    ENVELOPE_DECAY,
    ENVELOPE_SUSTAIN,
    ENVELOPE_RELEASE,
    ENVELOPE_SILENT
} envelope_stage;

typedef struct {
    float attack_time;
    float decay_time;
    float sustain_level;
    float release_time;
} envelope_settings;

typedef struct {
    envelope_stage stage;
    uint16_t cycles;
    uint32_t level;
    bool released;
} envelope_state;

// Main audio out buffer size in samples
#define AUDIO_OUT_BUFFER_SIZE 4096

// TODO determine voice buffer size
#define VOICE_BUFFER_SIZE (AUDIO_OUT_BUFFER_SIZE / 2)

#define OSCILLATOR_COUNT 2

typedef struct {
    bool active;
    uint8_t note;
    uint8_t velocity;
    oscillator_state osc[OSCILLATOR_COUNT];
    envelope_state env;
    int16_t samples[VOICE_BUFFER_SIZE];
} voice_entry;

typedef struct {
    oscillator_settings osc[OSCILLATOR_COUNT];
    envelope_settings env;
} settings;

extern settings current_settings;
