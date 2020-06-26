#include <Kinematics.h>
#include <Odometry.h>
#include <PID.h>
#include <Romi32U4.h>
#include <unity.h>

Romi32U4Motors motors;
Romi32U4Encoders encoders;

#define WHEEL_SEPERATION 0.146     // 14.6mm = 0.146m
#define WHEEL_CIRCUMFERENCE 0.215  // 6.85cm * pi = 0.215m
#define TICKS_PER_REVOLUTION 1440.0

Kinematics kinematics(WHEEL_CIRCUMFERENCE, WHEEL_SEPERATION);
Odometry leftWheelOdom(TICKS_PER_REVOLUTION);
Odometry rightWheelOdom(TICKS_PER_REVOLUTION);
PID left_pid(-300, 300, 0.6, 0.3, 0.5);
PID right_pid(-300, 300, 0.6, 0.3, 0.5);

void testMotorEncoderLoop() {
    unsigned int left_encoder_count = encoders.getCountsLeft();
    unsigned int right_encoder_count = encoders.getCountsRight();

    // move motors in opposite directions
    for (int speed = 0; speed <= 400; speed++) {
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(-speed);
        delay(4);
    }
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);

    // encoders should have registered new ticks
    int left_delta = encoders.getCountsLeft() - left_encoder_count;
    int right_delta = encoders.getCountsRight() - right_encoder_count;
    TEST_ASSERT_GREATER_THAN(0, left_delta);
    TEST_ASSERT_LESS_THAN(0, right_delta);
}

void testForwardMotion() {
    // RPM needed to achieve 0.5 m/s
    Kinematics::rpm req_rpm = kinematics.rpmForMotion(0.215, 0);

    // attempt to reach target velocity in 1 second
    float left_rpm, right_rpm;
    unsigned long start_time = millis();
    while (millis() - start_time < 2000) {
        // read current RPM
        left_rpm = leftWheelOdom.getRPM(encoders.getCountsLeft(), millis());
        right_rpm = rightWheelOdom.getRPM(encoders.getCountsRight(), millis());

        // determing PID values to zero out error
        float left_input = left_pid.compute(req_rpm.left_motor, left_rpm);
        float right_input = right_pid.compute(req_rpm.right_motor, right_rpm);

        Serial.print("req: ");
        Serial.print(req_rpm.left_motor);
        Serial.print(" act: ");
        Serial.print(left_rpm);
        Serial.print(" ctrl: ");
        Serial.println(left_input);

        motors.setLeftSpeed(left_input);
        motors.setRightSpeed(right_input);

        delay(10);
    }
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);

    Serial.println("");
    Serial.print("LEFT req: ");
    Serial.print(req_rpm.left_motor);
    Serial.print("RPM, actual: ");
    Serial.print(left_rpm);
    Serial.println("RPM");

    Serial.print("RIGHT req: ");
    Serial.print(req_rpm.right_motor);
    Serial.print("RPM, actual: ");
    Serial.print(right_rpm);
    Serial.println("RPM");

    // did we minimize error?
    float left_error = req_rpm.left_motor - left_rpm;
    float right_error = req_rpm.right_motor - right_rpm;
    TEST_ASSERT_FLOAT_WITHIN(4, 0, left_error);
    TEST_ASSERT_FLOAT_WITHIN(4, 0, right_error);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
}

void loop() {
    // ledGreen(true);
    // RUN_TEST(testMotorEncoderLoop);
    // ledGreen(false);

    // delay(200);

    ledRed(true);
    RUN_TEST(testForwardMotion);
    ledRed(false);

    UNITY_END();

    while (1)
        ;
}
