#pragma once

#include <stdbool.h>

#include "structures.h"

typedef struct {
    float min;
    float max;
    float step;
    float* value;
    bool dirty;
} control;

typedef struct {
    control shape;
    control amplitude;
    control tune;
    control velocity_response;
} osc_controls;

typedef struct {
    control attack;
    control decay;
    control sustain;
    control release;
} env_controls;

typedef struct {
    osc_controls osc[OSCILLATOR_COUNT];
    env_controls env;
} control_set;

extern control_set controls;

void controls_init();

float controlPositionToValue(control* ctrl, float f);
float controlValueToPosition(control* ctrl);
