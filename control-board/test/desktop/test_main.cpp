#include <unity.h>

void test_basic() { TEST_ASSERT_EQUAL(true, true); }

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_basic);

    UNITY_END();

    return 0;
}
