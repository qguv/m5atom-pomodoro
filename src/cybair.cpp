#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SGP30.h>
#include <color.h>
#include "font.h"

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define BRIGHTNESS (8)
#define BEST_HUE (21845UL)
#define MIDDLE_HUE (11000UL)
#define WORST_HUE (0UL)
/*
#define BEST_CO2 (400UL)
#define WORST_CO2 (2000UL)
*/

/* milliseconds */
#define FRAME_TIME (16)

enum state {
	STATE_NULL = 0,
	STATE_WAIT,
	STATE_READ,
	STATE_DISPLAY,
};

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);
Adafruit_SGP30 sgp;

uint32_t co2_color, ethanol_color;
uint32_t frame, anim_start_frame;
uint32_t next_frame;
enum state cur_state, next_state;
int co2_digits[5];
int ethanol_digits[5];

#define ALEN(A) (sizeof (A) / sizeof (*(A)))

static void set_digits(int *digits, int x)
{
	int i = 0;

	if (x >= 10000) {
		digits[i++] = (x / 10000) % 10;
	} else {
		digits[4] = -1;
	}

	if (x >= 1000) {
		digits[i++] = (x / 1000) % 10;
	} else {
		digits[3] = -1;
	}

	if (x >= 100) {
		digits[i++] = (x / 100) % 10;
	} else {
		digits[2] = -1;
	}

	if (x >= 10) {
		digits[i++] = (x / 10) % 10;
	} else {
		digits[1] = -1;
	}

	if (x >= 1) {
		digits[i++] = x % 10;
	} else {
		digits[0] = -1;
	}
}

static bool sgp30_read(void)
{
	bool ok = sgp.IAQmeasure() && sgp.IAQmeasureRaw() && (sgp.eCO2 != 400 || sgp.TVOC != 0);
	if (ok) {
		set_digits(co2_digits, sgp.eCO2);
		set_digits(ethanol_digits, sgp.rawEthanol);
	} else {
		for (int i = 0; i < ALEN(co2_digits); i++) co2_digits[i] = 0;
		for (int i = 0; i < ALEN(ethanol_digits); i++) ethanol_digits[i] = 0;
	}
	return ok;
}

static void show_digit(int x, uint8_t font[10][ROWS], uint32_t color)
{
	if (x < 0) {
		return;
	}

	for (int row = 0; row < ROWS; row++) {
		uint8_t n = font[x][row];
		for (int col = 0; col < COLS; col++) {
			int i = col + row * COLS;
			if (n & (1U << (COLS - col - 1))) {
				strip.setPixelColor(i, color);
			}
		}
	}
}

void setup(void)
{
	frame = 0;
	cur_state = STATE_READ;

	strip.begin();

	uint32_t indigo = strip.Color(BRIGHTNESS / 4, 0, BRIGHTNESS);
	while (!sgp.begin()) {
			strip.clear();
			strip.fill(indigo);
			strip.show();
			delay(100);
	}

	/* wait until chip is ready */
	int m[12] = {2, 3, 4, 5, 13, 21, 29, 28, 27, 26, 18, 10};
	for (int f = 0; !sgp30_read(); f = (f + 1) % 12) {
		strip.clear();
		for (int row = 0; row < ROWS; row++) {
			for (int col = 0; col < COLS; col++) {
				int i = col + row * COLS;
				strip.setPixelColor(i, i == m[f] ? strip.Color(5, 5, 5)
						: i == m[(f + 11) % 12] ? strip.Color(2, 2, 3)
						: i == m[(f + 10) % 12] ? strip.Color(0, 1, 2)
						: i == m[(f + 9) % 12] ? strip.Color(0, 0, 1)
						: strip.Color(0, 0, 0));
			}
		}
		strip.show();
		delay(3 * FRAME_TIME);
	}

	next_frame = millis() + FRAME_TIME;
}

void loop(void)
{
	uint32_t t, d;
	int subframe, digit_to_show;

	if (cur_state == STATE_WAIT) {
		t = millis();
		if (t >= next_frame) {
			cur_state = next_state;
			next_frame = t + FRAME_TIME;
		}
		delay(2);
		return;
	}

	switch (cur_state) {
	case STATE_READ:
		if (sgp30_read()) {
			uint16_t co2_hue = sgp.eCO2 > 800
				? WORST_HUE
				: sgp.eCO2 > 600
				? BEST_HUE / 2
				: BEST_HUE;
			co2_color = strip.ColorHSV(co2_hue, 255, BRIGHTNESS);
			uint16_t ethanol_hue = sgp.rawEthanol > 19500
				? WORST_HUE
				: sgp.rawEthanol > 19000
				? BEST_HUE / 2
				: BEST_HUE;
			ethanol_color = strip.ColorHSV(ethanol_hue, 255, BRIGHTNESS);

			anim_start_frame = frame;
			cur_state = STATE_DISPLAY;
		}
		break;

	case STATE_DISPLAY:
		d = frame - anim_start_frame;

		/* after this frame, wait for the next one */
		next_state = cur_state;
		cur_state = STATE_WAIT;

		digit_to_show = d / 32;
		subframe = d % 32;

		/* turn lights off between digits */
		if (subframe == 29) {
			strip.clear();
			strip.show();

		/* show a digit */
		} else if (subframe == 0 && digit_to_show < ALEN(ethanol_digits)) {
			strip.clear();
			show_digit(ethanol_digits[digit_to_show], font, ethanol_color);
			strip.show();

		/* read another one */
		} else if (digit_to_show >= 8) {
			cur_state = STATE_READ;
		}

		frame++;
		break;
	}
}
