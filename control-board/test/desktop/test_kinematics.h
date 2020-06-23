#pragma once

#include <Kinematics.h>
#include <unity.h>

#define WHEEL_DISTANCE 0.4  // cm

Kinematics ki(WHEEL_DISTANCE);

void testVelocityCalculation() {
    ki.setTwistTarget(0.2, 0.3);
    Kinematics::velocities vel = ki.getVelocities();
    TEST_ASSERT_EQUAL_FLOAT(vel.right_wheel, 0.24);
    TEST_ASSERT_EQUAL_FLOAT(vel.left_wheel, 0.14);
}
