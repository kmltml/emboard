#pragma once

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"

extern UART_HandleTypeDef huart6;

extern TaskHandle_t midi_task_handle;

void midi_task(void* args);

void midiIRQ();
