#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <color.h>

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define POMO_WORK_MS (30 * 60 * 1000)
#define POMO_BREAK_MS (8 * 60 * 1000)
#define POMO_CYCLE_MS (POMO_WORK_MS + POMO_BREAK_MS)
#define POMO_ALERT_FLASH_ON_MS (20)
#define POMO_ALERT_FLASH_OFF_MS (80)
#define POMO_ALERT_FLASHES (4)
#define POMO_ALERT_BETWEEN_FLASHES_MS (400)
#define POMO_ALERT_REPETITIONS (3)

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);

bool working;
bool was_working;
int flashes;
int flash_groups;

void setup(void)
{
	working = true;
	was_working = true;
	flashes = 0;
	flash_groups = 0;
	strip.begin();
}

void loop(void)
{
	if (flashes) {
		flashes--;

		/* all on */
		strip.clear();
		for (int i = 0; i < ROWS * COLS; i++)
			strip.setPixelColor(i, working ? 60 : 0, 20, working ? 0 : 60);
		strip.show();
		delay(POMO_ALERT_FLASH_ON_MS);

		/* all off */
		strip.clear();
		strip.show();
		delay(POMO_ALERT_FLASH_OFF_MS);

		if (!flashes)
			delay(POMO_ALERT_BETWEEN_FLASHES_MS);

		return;
	}

	if (flash_groups) {
		flash_groups--;

		flashes = POMO_ALERT_FLASHES + 1;

		return;
	}

	uint32_t t = millis() % POMO_CYCLE_MS;

	working = t < POMO_WORK_MS;
	if (was_working != working) {
		was_working = working;

		flash_groups = POMO_ALERT_REPETITIONS;

		return;
	}

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
