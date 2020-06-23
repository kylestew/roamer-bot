#include <unity.h>

#include "test_kinematics.h"

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(testVelocityCalculation);

    UNITY_END();

    return 0;
}
