#include "gui.h"
#include "synthesizer.h"

#include <stdbool.h>

#include "cmsis_os.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

#define LCD_WIDTH RK043FN48H_WIDTH
#define LCD_HEIGHT RK043FN48H_HEIGHT

#define SLIDER_DISTANCE           110
#define SLIDER_BG_WIDTH           50
#define SLIDER_WIDTH              15
#define SLIDER_RADIUS_INACTIVE    15
#define SLIDER_RADIUS_ACTIVE      23
#define SLIDER_POS_MIN_Y          (30 + SLIDER_RADIUS_ACTIVE)
#define SLIDER_POS_MAX_Y          (250 - SLIDER_RADIUS_ACTIVE)
#define SLIDER_HEIGHT             (SLIDER_POS_MAX_Y - SLIDER_POS_MIN_Y)
#define BACK_BUTTON_MARGIN        5
#define BACK_BUTTON_WIDTH         80
#define BACK_BUTTON_ARROW_OFFSET  10
#define MINI_BORDER_INACTIVE      5
#define MINI_BORDER_ACTIVE        10
#define MINI_BORDER_OFFSET        (MINI_BORDER_ACTIVE - MINI_BORDER_INACTIVE) / 2

static uint32_t lcd_image_fg[LCD_HEIGHT][LCD_WIDTH] __attribute__((section(".sdram"), unused));

#define COLOR_BG       0xffecfcff
#define COLOR_ACTIVE   0xff3e64ff
#define COLOR_INACTIVE 0xffb2fcff
#define COLOR_BACK     0xff00ff00

#define SELECTION_NONE (uint8_t)(-1)
#define SELECTION_BACK (SELECTION_NONE - 1)

typedef struct Rect {
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
} Rect;

typedef struct Slider {
	uint16_t posX;

	float min;
	float max;
	float* value;
	float step;
	bool mutable;
} Slider;

typedef struct ConfigPanel {
	Rect bounds;
	Slider* sliders;
	uint8_t sliderCount;
	uint8_t highlightedSlider;
} ConfigPanel;

typedef struct MainScreen {
	ConfigPanel* panels;
	uint8_t panelCount;
	uint8_t highlightedPanel;
} MainScreen;

//GUI states:
void viewMainScreen(MainScreen* screen);
void viewPanelScreen(ConfigPanel* panel);

//Displaying graphics elements:
void drawMainScreen(const MainScreen* mainScreen);
void drawMiniPanel(const MainScreen* mainScreen, uint8_t panelId);
void drawMiniSlider(const ConfigPanel* panel, uint8_t sliderId);
void drawMiniBackButton(const Rect* bounds);

void drawConfigPanel(const ConfigPanel* panel);
void drawSlider(const ConfigPanel* panel, uint8_t sliderId);
void drawBackButton();

//Resolving touch events:
uint8_t resolveSelectedPanel(const MainScreen* mainScreen, uint16_t touchX, uint16_t touchY);
uint8_t resolveSelectedSlider(const ConfigPanel* panel, uint16_t touchX, uint16_t touchY);
void waitUntilTouchStops(TS_StateTypeDef* touchScreenState);

//Slider calculations:
float sliderPositionToValue(const Slider* slider, uint16_t posY);
uint16_t sliderValueToPosition(const Slider* slider);
float clampf(float min, float x, float max);

//Math utils:
uint16_t round_to_uint16(float f);

void gui_init() {
	//Initialize LCD:
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(0, (uint32_t) lcd_image_fg);

	BSP_LCD_DisplayOn();

	BSP_LCD_SelectLayer(0);
	BSP_LCD_Clear(0xffff00ff);

	BSP_LCD_SetTransparency(0, 255);

	//Initialize touch screen:
	BSP_TS_Init(LCD_WIDTH, LCD_HEIGHT);
}

void gui_task(void* args) {

	//gui_init();

	//Sample values:
	float a = 60.0f;
    float b = 400.0f;
	float c = 7.0f;

	Slider sliders[3];
	uint8_t sliderCount = sizeof(sliders) / sizeof(sliders[0]);

	sliders[0].posX = 40.0f;
	sliders[0].min = 10.0f;
	sliders[0].max = 110.0f;
	sliders[0].step = 20.0f;
	sliders[0].value = &a;

	sliders[1].posX = sliders[0].posX + SLIDER_DISTANCE;
	sliders[1].min = 0.0f;
	sliders[1].max = 1000.0f;
	sliders[1].step = 100.0f;
	sliders[1].value = &b;

	sliders[2].posX = sliders[1].posX + SLIDER_DISTANCE;
	sliders[2].min = 3.0f;
	sliders[2].max = 9.0f;
	sliders[2].step = 1.0f;
	sliders[2].value = &c;

	ConfigPanel panels[1];
	uint8_t panelCount = sizeof(panels) / sizeof(panels[0]);

	panels[0].bounds.x = 20;
	panels[0].bounds.y = 20;
	panels[0].bounds.w = 140;
	panels[0].bounds.h = 80;
	panels[0].sliders = sliders;
	panels[0].sliderCount = sliderCount;
	panels[0].highlightedSlider = SELECTION_NONE;

	MainScreen mainScreen;
	mainScreen.panels = panels;
	mainScreen.panelCount = panelCount;
	mainScreen.highlightedPanel = SELECTION_NONE;

	viewMainScreen(&mainScreen);
}

void viewMainScreen(MainScreen* screen) {
	screen->highlightedPanel = SELECTION_NONE;

	drawMainScreen(screen);

	TS_StateTypeDef touchScreenState;
	while(true) {
		if(BSP_TS_GetState(&touchScreenState) == TS_OK) {
			if(touchScreenState.touchDetected > 0) {
				uint16_t touchX = touchScreenState.touchX[0];
				uint16_t touchY = touchScreenState.touchY[0];

				uint8_t selectedPanel = resolveSelectedPanel(screen, touchX, touchY);
				if(selectedPanel == screen->highlightedPanel) {
					if(selectedPanel != SELECTION_NONE) {
						waitUntilTouchStops(&touchScreenState);

						viewPanelScreen(&screen->panels[selectedPanel]);

						drawMainScreen(screen);
					}
				}
				else {
					uint8_t previouslyHighlightedPanel = screen->highlightedPanel;
					screen->highlightedPanel = selectedPanel;

					drawMiniPanel(screen, previouslyHighlightedPanel);
					drawMiniPanel(screen, selectedPanel);

					waitUntilTouchStops(&touchScreenState);
				}
			}
		}
	}
}

void viewPanelScreen(ConfigPanel* panel) {
	drawConfigPanel(panel);

	TS_StateTypeDef touchScreenState;
	while(true) {
		if(BSP_TS_GetState(&touchScreenState) == TS_OK) {
			if(touchScreenState.touchDetected > 0) {
				uint16_t touchX = touchScreenState.touchX[0];
				uint16_t touchY = touchScreenState.touchY[0];

				uint8_t selectedSlider = resolveSelectedSlider(panel, touchX, touchY);

				if(selectedSlider == SELECTION_BACK) {
					waitUntilTouchStops(&touchScreenState);
					return;
				}

				if(selectedSlider != panel->highlightedSlider) {
					uint8_t previouslyHighlightedSlider = panel->highlightedSlider;
					panel->highlightedSlider = selectedSlider;

					drawSlider(panel, previouslyHighlightedSlider);
				}

				if(selectedSlider != SELECTION_NONE) {
					Slider* slider = &panel->sliders[selectedSlider];
					float newValue = sliderPositionToValue(slider, touchY);
					*(slider->value) = newValue;
				}

				drawSlider(panel, panel->highlightedSlider);
			}
		}
	}
}

void waitUntilTouchStops(TS_StateTypeDef* touchScreenState) {
	while(BSP_TS_GetState(touchScreenState) != TS_OK || touchScreenState->touchDetected > 0);
}

void drawMainScreen(const MainScreen* mainScreen) {
	BSP_LCD_Clear(COLOR_BG);

	for(uint8_t i = 0; i < mainScreen->panelCount; ++i)
		drawMiniPanel(mainScreen, i);
}

void drawMiniPanel(const MainScreen* mainScreen, uint8_t panelId) {
	if(panelId != SELECTION_NONE && panelId != SELECTION_BACK) {
		const ConfigPanel* panel = &mainScreen->panels[panelId];

		if(mainScreen->highlightedPanel == panelId) {
			BSP_LCD_SetTextColor(COLOR_ACTIVE);
			BSP_LCD_FillRect(
				panel->bounds.x - MINI_BORDER_ACTIVE,
				panel->bounds.y - MINI_BORDER_ACTIVE,
				panel->bounds.w + 2 * MINI_BORDER_ACTIVE,
				panel->bounds.h + 2 * MINI_BORDER_ACTIVE
			);
		}
		else {
			BSP_LCD_SetTextColor(COLOR_BG);
			BSP_LCD_FillRect(
				panel->bounds.x - MINI_BORDER_ACTIVE,
				panel->bounds.y - MINI_BORDER_ACTIVE,
				panel->bounds.w + 2 * MINI_BORDER_ACTIVE,
				panel->bounds.h + 2 * MINI_BORDER_ACTIVE
			);

			BSP_LCD_SetTextColor(COLOR_INACTIVE);
			BSP_LCD_FillRect(
				panel->bounds.x - MINI_BORDER_INACTIVE - MINI_BORDER_OFFSET,
				panel->bounds.y - MINI_BORDER_INACTIVE - MINI_BORDER_OFFSET,
				panel->bounds.w + 2 * (MINI_BORDER_INACTIVE + MINI_BORDER_OFFSET),
				panel->bounds.h + 2 * (MINI_BORDER_INACTIVE + MINI_BORDER_OFFSET)
			);
		}

		BSP_LCD_SetTextColor(COLOR_BG);
		BSP_LCD_FillRect(panel->bounds.x, panel->bounds.y, panel->bounds.w, panel->bounds.h);

		for(uint8_t i = 0; i < panel->sliderCount; ++i)
			drawMiniSlider(panel, i);

		drawMiniBackButton(&panel->bounds);
	}
}

void drawConfigPanel(const ConfigPanel* panel) {
	BSP_LCD_Clear(COLOR_BG);

	for(uint8_t i = 0; i < panel->sliderCount; ++i)
		drawSlider(panel, i);

	drawBackButton();
}

void drawBackButton() {
	BSP_LCD_SetTextColor(COLOR_BACK);
	BSP_LCD_FillRect(
		LCD_WIDTH - BACK_BUTTON_WIDTH - BACK_BUTTON_MARGIN,
		BACK_BUTTON_MARGIN,
		BACK_BUTTON_WIDTH,
		SLIDER_POS_MIN_Y - 2 * BACK_BUTTON_MARGIN
	);

	BSP_LCD_SetTextColor(0xff000000);
	BSP_LCD_DrawHLine(
		LCD_WIDTH - BACK_BUTTON_WIDTH,
		SLIDER_POS_MIN_Y / 2,
		BACK_BUTTON_WIDTH - 2 * BACK_BUTTON_MARGIN
	);
	BSP_LCD_DrawLine(
		LCD_WIDTH - BACK_BUTTON_WIDTH,
		SLIDER_POS_MIN_Y / 2,
		LCD_WIDTH - BACK_BUTTON_WIDTH + BACK_BUTTON_ARROW_OFFSET,
		SLIDER_POS_MIN_Y / 2 - BACK_BUTTON_ARROW_OFFSET
	);
	BSP_LCD_DrawLine(
		LCD_WIDTH - BACK_BUTTON_WIDTH,
		SLIDER_POS_MIN_Y / 2,
		LCD_WIDTH - BACK_BUTTON_WIDTH + BACK_BUTTON_ARROW_OFFSET,
		SLIDER_POS_MIN_Y / 2 + BACK_BUTTON_ARROW_OFFSET
	);
}

void drawMiniBackButton(const Rect* bounds) {
	float scaleX = ((float) bounds->w) / LCD_WIDTH;
	float scaleY = ((float) bounds->h) / LCD_HEIGHT;

	BSP_LCD_SetTextColor(COLOR_BACK);
	BSP_LCD_FillRect(
		round_to_uint16(bounds->x + bounds->w - scaleX * BACK_BUTTON_WIDTH - scaleX * BACK_BUTTON_MARGIN),
		round_to_uint16(bounds->y + scaleY * BACK_BUTTON_MARGIN),
		round_to_uint16(scaleX * BACK_BUTTON_WIDTH),
		round_to_uint16(scaleY * (SLIDER_POS_MIN_Y - 2 * BACK_BUTTON_MARGIN))
	);

	BSP_LCD_SetTextColor(0xff000000);
	BSP_LCD_DrawHLine(
		round_to_uint16(bounds->x + bounds->w - scaleX * BACK_BUTTON_WIDTH),
		round_to_uint16(bounds->y + scaleY * SLIDER_POS_MIN_Y / 2),
		round_to_uint16(scaleX * (BACK_BUTTON_WIDTH - 2 * BACK_BUTTON_MARGIN))
	);
	BSP_LCD_DrawLine(
		round_to_uint16(bounds->x + bounds->w - scaleX * BACK_BUTTON_WIDTH),
		round_to_uint16(bounds->y + scaleY * SLIDER_POS_MIN_Y / 2),
		round_to_uint16(bounds->x + bounds->w - scaleX * (BACK_BUTTON_WIDTH - BACK_BUTTON_ARROW_OFFSET)),
		round_to_uint16(bounds->y + scaleY * (SLIDER_POS_MIN_Y / 2 - BACK_BUTTON_ARROW_OFFSET))
	);
	BSP_LCD_DrawLine(
		round_to_uint16(bounds->x + bounds->w - scaleX * BACK_BUTTON_WIDTH),
		round_to_uint16(bounds->y + scaleY * SLIDER_POS_MIN_Y / 2),
		round_to_uint16(bounds->x + bounds->w - scaleX * (BACK_BUTTON_WIDTH - BACK_BUTTON_ARROW_OFFSET)),
		round_to_uint16(bounds->y + scaleY * (SLIDER_POS_MIN_Y / 2 + BACK_BUTTON_ARROW_OFFSET))
	);
}

void drawSlider(const ConfigPanel* panel, uint8_t sliderId) {
	if(sliderId != SELECTION_NONE && sliderId != SELECTION_BACK) {
		const Slider* slider = &panel->sliders[sliderId];

		//Refresh background:
		BSP_LCD_SetTextColor(COLOR_BG);
		BSP_LCD_FillRect(
			slider->posX - SLIDER_BG_WIDTH / 2,
			SLIDER_POS_MIN_Y - SLIDER_RADIUS_ACTIVE,
			SLIDER_BG_WIDTH,
			SLIDER_HEIGHT + 2 * SLIDER_RADIUS_ACTIVE + 1
		);

		//Choose color palette:
		uint32_t primaryColor;
		uint16_t radius;

		if(panel->highlightedSlider == sliderId) {
			primaryColor = COLOR_ACTIVE;
			radius = SLIDER_RADIUS_ACTIVE;
		}
		else {
			primaryColor = COLOR_INACTIVE;
			radius = SLIDER_RADIUS_INACTIVE;
		}

		//Draw bar:
		BSP_LCD_SetTextColor(primaryColor);
		BSP_LCD_FillRect(
			slider->posX - SLIDER_WIDTH / 2,
			SLIDER_POS_MIN_Y,
			SLIDER_WIDTH,
			SLIDER_HEIGHT
		);

		//Draw knob:
		float posY = sliderValueToPosition(slider);

		BSP_LCD_SetTextColor(0xff5edfff);
		BSP_LCD_FillCircle(slider->posX, posY, radius);

		BSP_LCD_SetTextColor(0xffecfcff);
		BSP_LCD_FillCircle(slider->posX, posY, radius * 83/100);

		BSP_LCD_SetTextColor(primaryColor);
		BSP_LCD_FillCircle(slider->posX, posY, radius / 2);
	}
}

void drawMiniSlider(const ConfigPanel* panel, uint8_t sliderId) {
	const Rect* bounds = &panel->bounds;
	const Slider* slider = &panel->sliders[sliderId];

	float scaleX = ((float) bounds->w) / LCD_WIDTH;
	float scaleY = ((float) bounds->h) / LCD_HEIGHT;
	float scaleR = (scaleX + scaleY) / 2;

	//Choose color palette:
	uint32_t primaryColor;
	uint16_t radius;

	if(panel->highlightedSlider == sliderId) {
		primaryColor = COLOR_ACTIVE;
		radius = SLIDER_RADIUS_ACTIVE;
	}
	else {
		primaryColor = COLOR_INACTIVE;
		radius = SLIDER_RADIUS_INACTIVE;
	}
	radius = round_to_uint16(scaleR * radius);

	//Draw bar:
	BSP_LCD_SetTextColor(primaryColor);
	BSP_LCD_FillRect(
		round_to_uint16(bounds->x + scaleX * slider->posX - scaleX * SLIDER_WIDTH / 2),
		round_to_uint16(bounds->y + scaleY * SLIDER_POS_MIN_Y),
		round_to_uint16(scaleX * SLIDER_WIDTH),
		round_to_uint16(scaleY * SLIDER_HEIGHT)
	);

	//Draw knob:
	uint16_t posX = round_to_uint16(bounds->x + scaleX * slider->posX);
	uint16_t posY = round_to_uint16(bounds->y + scaleY * sliderValueToPosition(slider));

	BSP_LCD_SetTextColor(0xff5edfff);
	BSP_LCD_FillCircle(posX, posY, (uint16_t) radius);

	BSP_LCD_SetTextColor(0xffecfcff);
	BSP_LCD_FillCircle(posX, posY, (uint16_t) (radius * 0.83f));

	BSP_LCD_SetTextColor(primaryColor);
	BSP_LCD_FillCircle(posX, posY, (uint16_t) (radius * 0.5f));
}

uint8_t resolveSelectedPanel(const MainScreen* mainScreen, uint16_t touchX, uint16_t touchY) {
	for(uint8_t i = 0; i < mainScreen->panelCount; ++i) {
		const Rect* bounds = &mainScreen->panels[i].bounds;
		if(touchX >= bounds->x && touchX <= bounds->x + bounds->w) {
			if(touchY >= bounds->y && touchY <= bounds->y + bounds->h) {
				return i;
			}
		}
	}
	return SELECTION_NONE;
}

uint8_t resolveSelectedSlider(const ConfigPanel* panel, uint16_t touchX, uint16_t touchY) {
	if(touchX + BACK_BUTTON_MARGIN >= LCD_WIDTH - BACK_BUTTON_WIDTH && touchY + BACK_BUTTON_MARGIN <= SLIDER_POS_MIN_Y)
		return SELECTION_BACK;

	if(touchY >= SLIDER_POS_MIN_Y - SLIDER_RADIUS_ACTIVE && touchY <= SLIDER_POS_MAX_Y + SLIDER_RADIUS_ACTIVE) {
		for(uint8_t i = 0; i < panel->sliderCount; ++i) {
			uint16_t posX = panel->sliders[i].posX;
			if(touchX > posX - SLIDER_BG_WIDTH && touchX < posX + SLIDER_BG_WIDTH)
				return i;
		}
	}
	return SELECTION_NONE;
}

float sliderPositionToValue(const Slider* slider, uint16_t posY) {
	float value = slider->max - (posY - SLIDER_POS_MIN_Y) * (slider->max - slider->min) / SLIDER_HEIGHT;
	value = slider->min + round_to_uint16((value - slider->min) / slider->step) * slider->step; //?
	return clampf(slider->min, value, slider->max);
}

uint16_t sliderValueToPosition(const Slider* slider) {
	return (uint16_t) (SLIDER_POS_MIN_Y + SLIDER_HEIGHT * (slider->max - *(slider->value)) / (slider->max - slider->min));
}

float clampf(float min, float x, float max) {
	return (x < min) ? min : ((x > max) ? max : x);
}

uint16_t round_to_uint16(float f) {
  if(f < 0.0f)
    return 0;

  uint16_t d = f;
  if(f - d >= 0.5f) ++d;
  return d;
}
