#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <color.h>

#define LED_PIN (27)
#define BUTTON_PIN (39)
#define COLS (5)
#define ROWS (5)
#define ON_DELAY_THRESHHOLD (10)

#define POMO_WORK_MS (47 * 60 * 1000)
#define POMO_BREAK_MS (13 * 60 * 1000)
#define POMO_CYCLE_MS (POMO_WORK_MS + POMO_BREAK_MS)
#define POMO_ALERT_FLASH_ON_MS (20)
#define POMO_ALERT_FLASH_OFF_MS (80)
#define POMO_ALERT_FLASHES (4)
#define POMO_ALERT_BETWEEN_FLASHES_MS (400)
#define POMO_ALERT_REPETITIONS (10)

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);

bool was_working;
uint32_t t0;
int flash_color[3];
int flashes;
int flash_groups;
int on_delay;

void setup(void)
{
	was_working = true;
	t0 = 0;
	flashes = 0;
	flash_groups = 0;
	on_delay = 0;

	strip.begin();

	pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop(void)
{
	/* debounce */
	if (digitalRead(BUTTON_PIN) == LOW) {
		on_delay++;
	} else if (on_delay > 0) {
		on_delay--;
	}

	if (flashes) {
		flashes--;

		/* all on */
		strip.clear();
		for (int i = 0; i < ROWS * COLS; i++)
			strip.setPixelColor(i, flash_color[0], flash_color[1], flash_color[2]);
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
		if (flash_groups > 0)
			flash_groups--;
		flashes = POMO_ALERT_FLASHES + 1;
		return;
	}

	uint32_t t_raw = millis();

	/* button pressed: switch break/work */
	if (on_delay >= ON_DELAY_THRESHHOLD) {
		on_delay = 0;
		was_working = !was_working;

		t0 = t_raw;
		if (was_working) {
			t0 -= POMO_WORK_MS;
		}
	}

	uint32_t t = (t_raw - t0) % POMO_CYCLE_MS;

	const bool working = t < POMO_WORK_MS;
	if (was_working != working) {
		was_working = working;

		flash_groups = POMO_ALERT_REPETITIONS;
		flash_color[0] = working ? 60 : 0;
		flash_color[1] = 20;
		flash_color[2] = working ? 0 : 60;

		return;
	}

	uint8_t completion = working
		? (ROWS * COLS) * t / POMO_WORK_MS
		: (ROWS * COLS) - 1 - (ROWS * COLS) * (t - POMO_WORK_MS) / POMO_BREAK_MS;

	strip.clear();

	for (int c = 0; c < COLS; c++) {
		for (int r = 0; r < ROWS; r++) {
			int i = r + c * ROWS;
			if (i <= completion) {
				strip.setPixelColor(i, working ? 20 : 0, 6, working ? 0 : 20);
			}
		}
	}

	strip.show();

	delay(10);
}
