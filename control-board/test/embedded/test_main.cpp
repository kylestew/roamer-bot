#include <Romi32U4.h>
#include <unity.h>

void test_basic() { TEST_ASSERT_EQUAL(1, 1); }

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_basic);
}

void loop() {
    ledGreen(true);
    delay(500);
    ledGreen(false);
    delay(500);
    UNITY_END();
}
