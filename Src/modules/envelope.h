#pragma once

#include <stdint.h>

#include "structures.h"

void envelope_init();

void envelope_reset(voice_entry* voice);

void envelope_process(voice_entry* voice);
