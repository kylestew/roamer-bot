#pragma once

#include <Kinematics.h>
#include <unity.h>

#define WHEEL_DISTANCE 0.4  // meters
#define WHEEL_CIRCUMFERENCE 0.5

Kinematics ki(WHEEL_CIRCUMFERENCE, WHEEL_DISTANCE);

void testVelocityCalculation() {
    Kinematics::rpm prm = ki.rpmForMotion(0.2, 0.3);

    // velocity = distance center of axel needs to travel / second
    float left_vel = 0.24;
    float right_vel = 0.14;

    // velocity / distance wheel travels = rot / second
    float left_rpm = left_vel / WHEEL_CIRCUMFERENCE;
    float right_rpm = right_vel / WHEEL_CIRCUMFERENCE;

    TEST_ASSERT_EQUAL_FLOAT(vel.left_motor, left_rpm * RPM_TO_RPS);
    TEST_ASSERT_EQUAL_FLOAT(vel.right_motor, right_rpm * RPM_TO_RPS);
}
