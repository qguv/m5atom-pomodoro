#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <color.h>

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define POMO_WORK_MS (25 * 60 * 1000)
#define POMO_BREAK_MS (5 * 60 * 1000)
#define POMO_CYCLE_MS (POMO_WORK_MS + POMO_BREAK_MS)

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);

void setup(void)
{
	strip.begin();
}

void loop(void)
{
	uint32_t t = millis() % POMO_CYCLE_MS;

	bool working = t < POMO_WORK_MS;
	uint8_t completion = working
		? (ROWS * COLS) * t / POMO_WORK_MS
		: (ROWS * COLS) - 1 - (ROWS * COLS) * (t - POMO_WORK_MS) / POMO_BREAK_MS;

	strip.clear();

	for (int c = 0; c < COLS; c++) {
		for (int r = 0; r < ROWS; r++) {
			int i_anim = r + c * ROWS;
			int i_strip = c + r * COLS;
			if (i_anim <= completion) {
				strip.setPixelColor(i_strip, working ? 6 : 0, 2, working ? 0 : 6);
			}
		}
	}
	strip.show();

	delay(10);
}
