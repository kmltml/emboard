#include "main.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "stm32746g_discovery_lcd.h"
#include <stdbool.h>

#define LCD_WIDTH RK043FN48H_WIDTH
#define LCD_HEIGHT RK043FN48H_HEIGHT

static uint32_t lcd_image_fg[LCD_HEIGHT][LCD_WIDTH] __attribute__((section(".sdram"), unused));

void lcd_init(void) {
  uint8_t lcd_status = BSP_LCD_Init();

  /* Initialize the LCD Layers */
  BSP_LCD_LayerDefaultInit(0, (uint32_t) lcd_image_fg);

  BSP_LCD_DisplayOn();

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(0xffff00ff);

  BSP_LCD_SetTransparency(0, 255);
}

void mainTask(void* _) {
  lcd_init();
  while(true) {
    BSP_LCD_SetTextColor(0xff00ff00);
    BSP_LCD_DrawRect(10, 10, 20, 20);
    for(size_t i = 0; i < 100; i++) {
      lcd_image_fg[i][50] = 0xff000000;
    }
    osDelay(1000);
  }
}
