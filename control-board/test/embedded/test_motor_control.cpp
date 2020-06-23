#include <Kinematics.h>
#include <Romi32U4.h>
#include <unity.h>

#define WHEEL_DISTANCE 0.4  // cm

Kinematics ki(WHEEL_DISTANCE);

void testForwardMotion() {
    ki.setTwistTarget(0.25, 0.0);

    // TODO: assert motion achieved

    TEST_ASSERT_EQUAL(0, 1);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    ledGreen(true);
}

void loop() {
    RUN_TEST(testForwardMotion);
    delay(500);

TODO:
    setup a motor control scenario and see that it works

        ledGreen(false);
    UNITY_END();
}
