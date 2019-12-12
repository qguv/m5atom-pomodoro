#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SGP30.h>
#include <Wire.h>
#include <color.h>

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define BRIGHTNESS (32)
#define BEST_HUE (21845UL)
#define WORST_HUE (0UL)
#define BEST_CO2 (400UL)
#define WORST_CO2 (2000UL)

/* milliseconds */
#define FRAME_TIME (16)

static uint8_t font[10][4] = {
	{
		0b00001110,
		0b00010001,
		0b00010001,
		0b00001110,
	},
	{
		0b00010010,
		0b00010001,
		0b00011111,
		0b00010000,
	},
	{
		0b00011000,
		0b00010101,
		0b00010101,
		0b00010010,
	},
	{
		0b00001010,
		0b00010001,
		0b00010101,
		0b00001110,
	},
	{
		0b00000110,
		0b00000101,
		0b00000100,
		0b00011111,
	},
	{
		0b00010111,
		0b00010101,
		0b00010101,
		0b00001001,
	},
	{
		0b00001110,
		0b00010101,
		0b00010101,
		0b00001000,
	},
	{
		0b00010001,
		0b00001001,
		0b00000101,
		0b00000011,
	},
	{
		0b00001010,
		0b00010101,
		0b00010101,
		0b00001010,
	},
	{
		0b00000010,
		0b00000101,
		0b00000101,
		0b00011110,
	},
};

enum state {
	STATE_NULL = 0,
	STATE_WAIT,
	STATE_READ,
	STATE_DISPLAY,
};

Adafruit_NeoPixel strip(COLS * ROWS, LED_PIN);
Adafruit_SGP30 sgp;

uint32_t co2_color, off, indigo, red, max_red;
uint32_t frame, anim_start_frame;
uint32_t next_frame;
enum state cur_state, next_state;
int co2_digits[5];

static bool sgp30_read(void)
{
	return sgp.IAQmeasure() && sgp.IAQmeasureRaw() && (sgp.eCO2 != 400 || sgp.TVOC != 0);
}

static void show_digit(int x)
{
	if (x < 0) {
		return;
	}

	Serial.printf("%d:\r\n", x);
	for (int row = 0; row < ROWS; row++) {
		uint8_t n = font[x][row];
		for (int col = 0; col < COLS; col++) {
			bool show = n & (1U << (COLS - col - 1));
			int i = col + row * COLS;
			strip.setPixelColor(i, show ? co2_color : off);
			Serial.printf("%s ", show ? "*" : " ");
		}
		Serial.println();
	}
	Serial.println();
}

void calc_digits(int *digits, int x)
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

void setup(void)
{
	Serial.begin(9600);
	Serial.println();
	Serial.println("== begin ==");
	frame = 0;
	cur_state = STATE_READ;

	strip.begin();

	off = strip.Color(0, 0, 0);
	indigo = strip.gamma32(strip.ColorHSV(52000, 255, BRIGHTNESS));
	red = strip.gamma32(strip.ColorHSV(0, 255, BRIGHTNESS));
	max_red = strip.Color(255, 0, 0);

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
						: off);
			}
		}
		strip.show();
		delay(3 * FRAME_TIME);
	}

	next_frame = millis() + FRAME_TIME;
}

void loop(void)
{
	uint32_t t;
	uint32_t d, piece;

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
			Serial.println(sgp.eCO2);
			calc_digits(co2_digits, sgp.eCO2);
			uint16_t hue = map_hue(sgp.eCO2, BEST_CO2, WORST_CO2, BEST_HUE, WORST_HUE, false);
			co2_color = strip.gamma32(strip.ColorHSV(hue, 255, BRIGHTNESS));
			anim_start_frame = frame;
			cur_state = STATE_DISPLAY;
		}
		break;

	case STATE_DISPLAY:
		d = frame - anim_start_frame;

		/* after this frame, wait for the next one */
		next_state = cur_state;
		cur_state = STATE_WAIT;

		/* some gaps between the numbers */
		if (d % 32 < 28) {
			strip.clear();
			strip.show();

		/* show a digit */
		} else if ((piece = d / 32) < 5) {
			strip.clear();
			show_digit(co2_digits[piece]);
			strip.show();

		/* read another one */
		} else if (piece >= 8) {
			cur_state = STATE_READ;
		}

		frame++;
		break;
	}
}
