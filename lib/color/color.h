#define MIN(X, Y) (((Y) < (X)) ? (Y) : (X))
#define MAX(X, Y) (((Y) > (X)) ? (Y) : (X))

/*
 * turn a value x (in the range best_x to worst_x) into a hue in the range
 * best_hue to worst_hue. set hue_crosses_zero if the range of hues that you
 * want contains the hue 0 somewhere in the middle
 */
uint16_t map_hue(int32_t x,
		int32_t best_x, int32_t worst_x,
		uint16_t best_hue, uint16_t worst_hue,
		bool hue_crosses_zero);
