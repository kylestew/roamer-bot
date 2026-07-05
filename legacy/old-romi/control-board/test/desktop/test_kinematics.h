#pragma once

#include <Kinematics.h>
#include <unity.h>

#define WHEEL_DISTANCE 0.4  // meters
#define WHEEL_CIRCUMFERENCE 0.5

Kinematics ki(WHEEL_CIRCUMFERENCE, WHEEL_DISTANCE);

void testLinearRPM() {
    Kinematics::rpm rpm = ki.rpmForMotion(0.2, 0.0);

    // velocity / distance wheel travels = wheel rotations / second
    float linear_vel = 0.2;
    float rps = linear_vel / WHEEL_CIRCUMFERENCE;
    float expected = rps * RPS_TO_RPM;

    TEST_ASSERT_EQUAL_FLOAT(rpm.left_motor, expected);
    TEST_ASSERT_EQUAL_FLOAT(rpm.right_motor, expected);
}

void testRotationalRPM() {
    Kinematics::rpm rpm = ki.rpmForMotion(0, 0.3);

    // linear velocity = radius * angular velocity
    float ang_v = 0.3;
    float lin_v = (WHEEL_DISTANCE / 2.0) * ang_v;

    // positive angular velocity = CW spin = left motor moving forward
    float left_rpm = -(lin_v / WHEEL_CIRCUMFERENCE) * RPS_TO_RPM;
    float right_rpm = (lin_v / WHEEL_CIRCUMFERENCE) * RPS_TO_RPM;

    TEST_ASSERT_EQUAL_FLOAT(rpm.left_motor, left_rpm);
    TEST_ASSERT_EQUAL_FLOAT(rpm.right_motor, right_rpm);
}

void testCombinedRPM() {
    Kinematics::rpm rpm = ki.rpmForMotion(0.2, 0.3);

    TEST_ASSERT_EQUAL_FLOAT(rpm.left_motor, 16.8);
    TEST_ASSERT_EQUAL_FLOAT(rpm.right_motor, 31.2);
}
