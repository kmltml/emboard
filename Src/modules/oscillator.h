#pragma once

#include "structures.h"

void oscillator_init();

void oscillator_reset(voice_entry* voice);

void oscillator_generate(voice_entry* voice);
