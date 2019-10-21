#pragma once
#include "voice_scheduler.h"

void synthesize(voice_entry* voice);

void mix();

void synthesizer_task(void* args);
