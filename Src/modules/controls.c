#include "controls.h"

control_set controls;

static void initOscillator(int oscIndex) {
    osc_controls* osc = &controls.osc[oscIndex];
    osc->shape = (control){
        .min = 0.1f,
        .max = 4.1f,
        .step = 1.0f,
        .value = &current_settings.osc[oscIndex].shape,
    };

    osc->amplitude = (control){
        .min = 0.0f,
        .max = 1.0f,
        .step = 0.0f,
        .value = &current_settings.osc[oscIndex].amplitude,
    };

    osc->tune = (control){
        .min = -12.0f,
        .max = 12.0f,
        .step = 1.0f,
        .value = &current_settings.osc[oscIndex].tune,
    };

    osc->velocity_response = (control){
        .min = 0.0f,
        .max = 1.0f,
        .step = 0.0f,
        .value = &current_settings.osc[oscIndex].velocity_response,
    };
}

static void initEnvelope() {
    env_controls* env = &controls.env;

    env->attack = (control){
        .min = 0.0,
        .max = 2.0,
        .value = &current_settings.env.attack_time,
        .step = 0.0,
    };
    env->decay = (control){
        .min = 0.0,
        .max = 2.0,
        .value = &current_settings.env.decay_time,
        .step = 0.0,
    };
    env->sustain = (control){
        .min = 0.0,
        .max = 1.0,
        .value = &current_settings.env.sustain_level,
        .step = 0.0,
    };
    env->release = (control){
        .min = 0.0,
        .max = 5.0,
        .value = &current_settings.env.release_time,
        .step = 0.0,
    };
}

void controls_init() {
    for (int i = 0; i < OSCILLATOR_COUNT; i++) {
        initOscillator(i);
    }
    initEnvelope();
}

static uint16_t round_to_uint16(float f) {
    if (f < 0.0f)
        return 0;

    uint16_t d = f;
    if (f - d >= 0.5f)
        ++d;
    return d;
}

float clampf(float min, float x, float max) {
    return (x < min) ? min : ((x > max) ? max : x);
}

float controlPositionToValue(control* ctrl, float f) {
    float v = f * ctrl->max + (1.0 - f) * ctrl->min;
    if (ctrl->step != 0.0) {
        v = ctrl->min +
            round_to_uint16((v - ctrl->min) / ctrl->step) * ctrl->step;
    }
    return clampf(ctrl->min, v, ctrl->max);
}

float controlValueToPosition(control* ctrl) {
    float v = (*ctrl->value - ctrl->min) / (ctrl->max - ctrl->min);
    return v;
}
