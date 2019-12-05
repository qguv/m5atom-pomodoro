#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_SGP30.h"

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define REST_TIME (10000)
#define DISPLAY_TIME (1000)
#define FRAME_TIME (50)

#define BRIGHTNESS (42)
#define BEST_HUE (21845UL)
#define WORST_HUE (0UL)
#define BEST_CO2 (400UL)
#define WORST_CO2 (2000UL)

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);
Adafruit_SGP30 sgp;

uint32_t off, indigo, red, max_red;
int reports;

static bool sgp30_read(void)
{
	return sgp.IAQmeasure() && sgp.IAQmeasureRaw() && (sgp.eCO2 != 400 || sgp.TVOC != 0);
}

void setup(void)
{
	strip.begin();

	reports = 0;

	off = strip.Color(0, 0, 0);
	indigo = strip.gamma32(strip.ColorHSV(52000, 255, BRIGHTNESS));
	red = strip.gamma32(strip.ColorHSV(0, 255, BRIGHTNESS));
	max_red = strip.Color(255, 0, 0);

	while (!sgp.begin()) {
		strip.fill(indigo);
		strip.show();
		delay(DISPLAY_TIME);
	}

	/* wait until chip is ready */
	int m[12] = {2, 3, 4, 5, 13, 21, 29, 28, 27, 26, 18, 10};
	for (int f = 0; !sgp30_read(); f = (f + 1) % 12) {
		for (int row = 0; row < ROWS; row++) {
			for (int col = 0; col < COLS; col++) {
				int i = col + row * COLS;
				strip.setPixelColor(i, i == m[f] ? strip.Color(5, 5, 5)
						: i == m[(f + 11) % 12] ? strip.Color(2, 2, 3)
						: i == m[(f + 10) % 12] ? strip.Color(0, 1, 2)
						: i == m[(f + 9) % 12] ? strip.Color(0, 0, 1)
						: off);
			}
		}
		strip.show();
		delay(3 * FRAME_TIME);
	}
}

void loop(void)
{
	while (!sgp30_read()) {
		delay(FRAME_TIME);
	}

	if (sgp.eCO2 > WORST_CO2) {
		for (int f = 0; f < 17; f++) {
			strip.fill(f % 8 ? red : max_red);
			strip.show();
			delay(FRAME_TIME);
		}
		strip.fill(red);
		strip.show();
		delay(REST_TIME);
		return;
	}

	uint32_t color;
	uint16_t hue = map(max(sgp.eCO2, (uint16_t) BEST_CO2), BEST_CO2, WORST_CO2, BEST_HUE, WORST_HUE);
	color = strip.gamma32(strip.ColorHSV(hue, 255, BRIGHTNESS));

	for (int animate_out = 0; animate_out < 2; animate_out++) {
		for (int sweep = 0; sweep < COLS; sweep++) {
			for (int col = 0; col < COLS; col++) {
				for (int row = 0; row < ROWS; row++) {
					int i = col + row * COLS;
					int on = animate_out ? col > sweep : col <= sweep;
					strip.setPixelColor(i, on ? color : off);
				}
			}

			strip.show();
			delay(FRAME_TIME);
		}
		reports++;
		delay(DISPLAY_TIME);
	}
	delay(REST_TIME);
}
