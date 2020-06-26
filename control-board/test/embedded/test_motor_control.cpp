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
#define MAX_RPM 200      // experimentally found safe upper limit
#define SAMPLE_DELTA 10  // 10 ms

Kinematics kinematics(WHEEL_CIRCUMFERENCE, WHEEL_SEPERATION);
Odometry leftWheelOdom(TICKS_PER_REVOLUTION);
Odometry rightWheelOdom(TICKS_PER_REVOLUTION);
PID left_pid(-MAX_RPM, MAX_RPM, 0.8, 12.0, 0, SAMPLE_DELTA / 1000.0);
PID right_pid(-MAX_RPM, MAX_RPM, 0.8, 12.0, 0, SAMPLE_DELTA / 1000.0);

void testMotorEncoderLoop() {
    unsigned int left_encoder_count = encoders.getCountsLeft();
    unsigned int right_encoder_count = encoders.getCountsRight();

    // your code must cap RPM output
    motors.allowTurbo(true);

    // move motors in opposite directions
    float power, left_rpm, right_rpm;
    for (int target_rpm = 0; target_rpm <= 200; target_rpm++) {
        // experimentally found rpm -> power required mapping
        // With proper tuning, the PID controller finds these
        power = 1.5 * target_rpm + 20;

        // apply actual
        motors.setLeftSpeed(power);
        motors.setRightSpeed(-power);

        // display current RPM
        left_rpm = leftWheelOdom.getRPM(encoders.getCountsLeft(), millis());
        right_rpm = rightWheelOdom.getRPM(encoders.getCountsRight(), millis());
        Serial.print(target_rpm);
        Serial.print(", ");
        Serial.print(power);
        Serial.print(", ");
        Serial.print(left_rpm);
        Serial.print(", ");
        Serial.println(right_rpm);

        delay(SAMPLE_DELTA);
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
    // attempt 0.25 m/s forward velocity in 1 second
    Kinematics::rpm req_rpm = kinematics.rpmForMotion(0.215, 0);

    float left_rpm, right_rpm;
    unsigned long start_time = millis();
    while (millis() - start_time < 1000) {
        // read current RPM
        left_rpm = leftWheelOdom.getRPM(encoders.getCountsLeft(), millis());
        right_rpm = rightWheelOdom.getRPM(encoders.getCountsRight(), millis());

        // determing PID values to zero out error
        float left_input = left_pid.calculate(req_rpm.left_motor, left_rpm);
        float right_input = right_pid.calculate(req_rpm.right_motor, right_rpm);

        Serial.print("req: ");
        Serial.print(req_rpm.left_motor);
        Serial.print(" act: ");
        Serial.print(left_rpm);
        Serial.print(" ctrl: ");
        Serial.println(left_input);

        motors.setLeftSpeed(left_input);
        motors.setRightSpeed(right_input);

        delay(SAMPLE_DELTA);
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
    TEST_ASSERT_FLOAT_WITHIN(3, 0, left_error);
    TEST_ASSERT_FLOAT_WITHIN(3, 0, right_error);
}

void testRotation() {
    // attempt to reach pi/2 rad/s in 1s
    Kinematics::rpm req_rpm = kinematics.rpmForMotion(0, 1.5707);

    float left_rpm, right_rpm;
    unsigned long start_time = millis();
    while (millis() - start_time < 1000) {
        // read current RPM
        left_rpm = leftWheelOdom.getRPM(encoders.getCountsLeft(), millis());
        right_rpm = rightWheelOdom.getRPM(encoders.getCountsRight(), millis());

        // determing PID values to zero out error
        float left_input = left_pid.calculate(req_rpm.left_motor, left_rpm);
        float right_input = right_pid.calculate(req_rpm.right_motor, right_rpm);

        motors.setLeftSpeed(left_input);
        motors.setRightSpeed(right_input);

        delay(SAMPLE_DELTA);
    }
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);

    // did we minimize error?
    float left_error = req_rpm.left_motor - left_rpm;
    float right_error = req_rpm.right_motor - right_rpm;
    TEST_ASSERT_FLOAT_WITHIN(3, 0, left_error);
    TEST_ASSERT_FLOAT_WITHIN(3, 0, right_error);
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

    delay(200);

    ledRed(true);
    RUN_TEST(testRotation);
    ledRed(false);

    UNITY_END();

    while (1)
        ;
}
