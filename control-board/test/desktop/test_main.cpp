#include <unity.h>

#include "test_kinematics.h"
#include "test_odometry.h"

int main(int argc, char** argv) {
    UNITY_BEGIN();

    // kinematics
    RUN_TEST(testVelocityCalculation);

    // odometry
    RUN_TEST(testSimpleOdometry);
    RUN_TEST(testMultipleOdomCalls);

    UNITY_END();

    return 0;
}
