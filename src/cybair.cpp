#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <color.h>
#include <FS.h>
#include <SPIFFS.h>

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define POMO_WORK_MS (47 * 60 * 1000)
#define POMO_BREAK_MS (13 * 60 * 1000)
#define POMO_CYCLE_MS (POMO_WORK_MS + POMO_BREAK_MS)
#define POMO_ALERT_FLASH_ON_MS (20)
#define POMO_ALERT_FLASH_OFF_MS (80)
#define POMO_ALERT_FLASHES (4)
#define POMO_ALERT_BETWEEN_FLASHES_MS (400)
#define POMO_ALERT_REPETITIONS (3)
#define BREAKFILE_PATH "/start_with_break"

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);

bool was_working;
bool start_with_break;
int flash_color[3];
int flashes;
int flash_groups;

void setup(void)
{
	start_with_break = false;
	was_working = true;
	flashes = 0;
	flash_groups = 0;
	flash_color[0] = 0xff;
	flash_color[1] = flash_color[2] = 0;
	strip.begin();

	bool needs_flash = !SPIFFS.begin(false);
	if (needs_flash) {

		// solid purple to show formatting
		strip.clear();
		for (int i = 0; i < ROWS * COLS; i++) {
			strip.setPixelColor(i, 0x10, 0, 0x20);
		}
		strip.show();

		// flash the filesystem
		bool ok = SPIFFS.begin(true);

		if (ok) {
			flash_groups = 1;
			flash_color[0] = 0;
			flash_color[1] = 0x10;
			flash_color[2] = 0;
		} else {
			flash_groups = -1;
			flash_color[0] = 0x10;
			flash_color[1] = 0;
			flash_color[2] = 0;
			return;
		}

	}

	if (SPIFFS.exists(BREAKFILE_PATH)) {
		SPIFFS.remove(BREAKFILE_PATH);
		start_with_break = true;
		was_working = false;

	} else {
		File f = SPIFFS.open(BREAKFILE_PATH, FILE_WRITE);
		f.print("\n");
		f.close();
	}
}

void loop(void)
{
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

	uint32_t t = millis();
	if (start_with_break) {
		t += POMO_WORK_MS;
	}
	t %= POMO_CYCLE_MS;

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
