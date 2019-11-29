#include "synthesizer.h"

#include "cmsis_os.h"
#include "stm32746g_discovery_audio.h"

settings current_settings;

typedef enum { BH_LOW, BH_HIGH } buffer_half;

buffer_half buffer_position = BH_LOW;

// For each sample we need one word for each channel
int16_t audio_out_buffer[AUDIO_OUT_BUFFER_SIZE * 4];

TaskHandle_t synthesizer_task_handle;

void BSP_AUDIO_OUT_TransferComplete_CallBack() {
    buffer_position = BH_HIGH;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(synthesizer_task_handle, &higherPriorityTaskWoken);

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
    buffer_position = BH_LOW;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(synthesizer_task_handle, &higherPriorityTaskWoken);

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void synthesizer_init() {
    uint8_t init_res =
        BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE1, 50, AUDIO_FREQUENCY_44K);
    if (init_res == 0) {
        printf("[INIT] Audio init ok\n");
    } else {
        printf("[INIT] Audio init error: %d\n", init_res);
    }
    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) audio_out_buffer,
                               sizeof(audio_out_buffer) / sizeof(uint32_t));
}

void synthesize(voice_entry* voice) {
    oscillator_generate(voice);
    envelope_process(voice);
}

int16_t add_saturate(int16_t a, int16_t b) {
    int32_t res = a + b;
    const int16_t max = 0x7fff;
    const int16_t min = -0x8000;
    if (res >= max) {
        res = max;
    }
    if (res <= min) {
        res = min;
    }
    return res;
}

void mix(int16_t* out_buffer) {
    for (size_t i = 0; i < AUDIO_OUT_BUFFER_SIZE * 2; i++) {
        out_buffer[i] = 0;
    }
    for (size_t voice = 0; voice < VOICE_COUNT; voice++) {
        if (voice_table[voice].active) {
            for (size_t i = 0; i < VOICE_BUFFER_SIZE; i++) {
                int16_t sample = voice_table[voice].samples[i];
                out_buffer[i * 4] = add_saturate(out_buffer[i * 4], sample);
                out_buffer[i * 4 + 2] =
                    add_saturate(out_buffer[i * 4 + 2], sample);
            }
        }
    }
}

void synthesizer_task(void* args) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (buffer_position == BH_HIGH) {
            // in the sample project this is not necessary, but for some reason
            // playback stops immediately when this is not present
            BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) audio_out_buffer,
                                       sizeof(audio_out_buffer) /
                                           sizeof(uint32_t));
        }

        for (size_t i = 0; i < VOICE_COUNT; i++) {
            if (voice_table[i].active) {
                synthesize(&voice_table[i]);
            }
        }
        mix(buffer_position == BH_LOW
                ? audio_out_buffer
                : audio_out_buffer + (AUDIO_OUT_BUFFER_SIZE * 2));
    }
}
