#include <Adafruit_NeoPixel.h>
#include "color.h"

/*
 * turn a value x (in the range best_x to worst_x) into a hue in the range
 * best_hue to worst_hue. set hue_crosses_zero if the range of hues that you
 * want contains the hue 0 somewhere in the middle
 */
uint16_t map_hue(int32_t x,
		const int32_t best_x, const int32_t worst_x,
		const uint16_t best_hue, const uint16_t worst_hue,
		const bool hue_crosses_zero)
{
	/* constrain x */
	int32_t smallest_x = MIN(best_x, worst_x);
	int32_t largest_x = MAX(best_x, worst_x);
	int32_t range_x = largest_x - smallest_x;
	x = MAX(x, smallest_x);
	x = MIN(x, largest_x);

	/* ensure smallest_x is best_x */
	if (best_x == largest_x) {
		x = largest_x - (x - smallest_x);
	}

	/* determine hue range */
	int32_t smallest_hue = MIN(best_hue, worst_hue);
	int32_t largest_hue = MAX(best_hue, worst_hue);
	int32_t range_hue = hue_crosses_zero
		? (1 + (int32_t) UINT16_MAX - largest_hue) + smallest_hue
		: largest_hue - smallest_hue;

	/* scale best_x -> worst_x to 0 -> range_hue */
	uint16_t addend = x - smallest_x * range_hue / range_x;

	return (hue_crosses_zero == (best_hue == smallest_hue))
		? best_hue - addend
		: best_hue + addend;
}
