#include <Kinematics.h>
#include <Romi32U4.h>
#include <unity.h>

#define WHEEL_DISTANCE 0.4  // cm

Kinematics ki(WHEEL_DISTANCE);

void testVelocityCalculation() {
    ki.setTwistTarget(0.2, 0.3);
    Kinematics::velocities vel = ki.getVelocities();
    TEST_ASSERT_EQUAL(vel.right_wheel, 0.26);
    TEST_ASSERT_EQUAL(vel.left_wheel, 0.14);
}

void testForwardMotion() {
    ki.setTwistTarget(0.25, 0.0);

    // TODO: assert motion achieved
    TEST_ASSERT_EQUAL(0, 1);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    ledGreen(true);

    RUN_TEST(testVelocityCalculation);
}

void loop() {
    RUN_TEST(testForwardMotion);
    delay(500);

TODO:
    setup a motor control scenario and see that it works

        ledGreen(false);
    UNITY_END();
}
