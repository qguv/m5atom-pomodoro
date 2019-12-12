#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SGP30.h>
#include <Wire.h>

uint8_t font[10][4] = {
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

#define LED_PIN (32)
#define COLS (8)
#define ROWS (4)

#define BRIGHTNESS (16)
#define BEST_HUE (21845UL)
#define WORST_HUE (0UL)
#define BEST_CO2 (400UL)
#define WORST_CO2 (2000UL)

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

uint32_t co2_color, off, indigo, red, max_red;
uint32_t frame, anim_start_frame;
uint32_t next_frame;
enum state cur_state, next_state;
int co2_digits[5];

static bool sgp30_read(void)
{
	return sgp.IAQmeasure() && sgp.IAQmeasureRaw() && (sgp.eCO2 != 400 || sgp.TVOC != 0);
}

void show_digit(int x)
{
	Serial.print("showing digit ");
	Serial.print(x);
	Serial.println("!");

	if (x < 0) {
		return;
	}

	for (int row = 0; row < ROWS; row++) {
		uint8_t m = font[x][row];
		for (int col = 0; col < COLS; col++) {
			strip.setPixelColor(col + row * COLS, m & (1U << col) ? indigo : off);
		}
	}
}

void calc_digits(int x, int *digits)
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
	Serial.println("Hello world");
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
	uint32_t d;

	strip.clear();

	switch (cur_state) {
	case STATE_WAIT:
		t = millis();
		if (t >= next_frame) {
			cur_state = next_state;
			next_frame = t + FRAME_TIME;
		}
		break;

	case STATE_READ:
		if (sgp30_read()) {
			uint16_t hue = map(max(sgp.eCO2, (uint16_t) BEST_CO2), BEST_CO2, WORST_CO2, BEST_HUE, WORST_HUE);
			co2_color = strip.gamma32(strip.ColorHSV(hue, 255, BRIGHTNESS));
			Serial.print("Calculating digits for eCO2 level ");
			Serial.print(sgp.eCO2);
			Serial.println("");
			calc_digits(sgp.eCO2, co2_digits);
			Serial.print("digits are: ");
			Serial.print(co2_digits[0]);
			Serial.print(" ");
			Serial.print(co2_digits[1]);
			Serial.print(" ");
			Serial.print(co2_digits[2]);
			Serial.print(" ");
			Serial.print(co2_digits[3]);
			Serial.print(" ");
			Serial.println(co2_digits[4]);
			anim_start_frame = frame;
			cur_state = STATE_DISPLAY;
		}
		break;

	case STATE_DISPLAY:
		d = frame - anim_start_frame;
		next_state = cur_state;

		/* some gaps between the numbers */
		if (d % 32 > 28) {
			; /* wait */

		} else if (d < 32 * 1) {
			show_digit(co2_digits[0]);

		} else if (d < 32 * 2) {
			show_digit(co2_digits[1]);

		} else if (d < 32 * 3) {
			show_digit(co2_digits[2]);

		} else if (d < 32 * 4) {
			show_digit(co2_digits[3]);

		} else if (d < 32 * 5) {
			show_digit(co2_digits[4]);

		} else if (d >= 256) {
			next_state = STATE_READ;
		}

		cur_state = STATE_WAIT;
		frame++;
		break;
	}

	strip.show();
	delay(10);
}
