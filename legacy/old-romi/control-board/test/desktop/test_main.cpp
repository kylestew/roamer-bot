#include <unity.h>

#include "test_kinematics.h"
#include "test_odometry.h"
#include "test_pid.h"

int main(int argc, char** argv) {
    UNITY_BEGIN();

    // kinematics
    RUN_TEST(testLinearRPM);
    RUN_TEST(testRotationalRPM);
    RUN_TEST(testCombinedRPM);

    // odometry
    RUN_TEST(testSimpleOdometry);
    RUN_TEST(testMultipleOdomCalls);

    // pid
    RUN_TEST(testPID);

    UNITY_END();

    return 0;
}
