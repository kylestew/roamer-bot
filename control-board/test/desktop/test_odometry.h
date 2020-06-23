#pragma once

#include <Odometry.h>
#include <unity.h>

// ticks per rotation
#define TICKS_PER_REVOLUTION 1440.0

Odometry od(TICKS_PER_REVOLUTION);

void testSimpleOdometry() {
    long int new_ticks = 124;  // ticks start at zero
    unsigned long dt = 44;     // time starts at zero

    // for a given number of ticks:
    // revolutions = ticks / ticks per rotation
    // RPM introduces time
    // RPS = revolutions / time
    // RPM = RPS + 60

    // RPMs = (124 / 1440) / 0.044s * 60s = ~117.4 RPM
    int expected = (124 / TICKS_PER_REVOLUTION) / (44 / 1000.0) * 60;

    int rpm = od.getRPM(new_ticks, dt);
    TEST_ASSERT_EQUAL_FLOAT(rpm, expected);
}

void testMultipleOdomCalls() {}
