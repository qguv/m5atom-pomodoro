#include <inttypes.h>
#include <unity.h>
#include <color.h>

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_positive);
    UNITY_END();
    return 0;
}

void test_positive(void)
{
    uint16_t cases[][2] = {
        {0, 0},
        {9, 90},
        {20, 90},
    };

    for (int i = 0; i < sizeof(cases) / sizeof(uint16_t) / 2; i++) {
        TEST_ASSERT_EQUAL_INT(cases[i][1], map_hue(cases[i][0], 0, 9, 0, 90, false));
    }

    return 0;
}
