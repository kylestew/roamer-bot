#include <Kinematics.h>
#include <Odometry.h>
#include <PID.h>
#include <PololuRPiSlave.h>
#include <Romi32U4.h>

#define DEBUG FALSE

// wheel characteristics
#define WHEEL_SEPERATION 0.146     // 146mm = 0.146m
#define WHEEL_CIRCUMFERENCE 0.215  // 6.85cm * pi = 0.215m

// motor characteristics
#define TICKS_PER_REVOLUTION 1440.0
#define MAX_RPM 200  // experimentally found safe upper limit

/* Low Battery Warning
 * The Raspberry Pi seems most supceptable to brown out so I'm focusing on its needs.
 * According to the Romi32U4 onboard regulator docs [https://www.pololu.com/product/2858],
 * a dropout voltage might approach 0.75v during peak loads on the regulator. In order to
 * maintain 5V out, 5.75v must be delivered by the batteries.
 * (I'm not sure if this is correct, but the robot dies pretty much after hitting this point)
 */
#define LOW_BATT_VALUE 6000  // 0.25v buffer

// update rates
#define SAMPLE_DELTA 20       // 50 hz
#define COMMAND_TIMEOUT 1000  // ms
#define IMU_PUB_RATE 50       // 20 hz
#define BATT_CHECK_RATE 8000  // every 8 seconds
#define DEBUG_RATE 500        // 2 hz

struct Data {
    // [input] LEDs
    // 0, 1, 2
    bool led_red, led_green, led_yellow;
    // [output] buttons
    // 3, 4, 5
    bool buttonA, buttonB, buttonC;
    // [output] battery level
    // 6-7
    uint16_t batteryMillivolts;

    // [input] twist command
    // 8
    bool new_twist_command;
    // 9-12
    float twist_linear_x;
    // 13-16
    float twist_angle_z;

    // [output] velocity
    // 17-20
    float vel_linear;
    // 21-24
    float vel_angular;
};

PololuRPiSlave<struct Data, 10> slave;

PololuBuzzer buzzer;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
Romi32U4ButtonC buttonC;
Romi32U4Encoders encoders;
Romi32U4Motors motors;

Kinematics kinematics(WHEEL_CIRCUMFERENCE, WHEEL_SEPERATION);
Odometry leftWheelOdom(TICKS_PER_REVOLUTION);
Odometry rightWheelOdom(TICKS_PER_REVOLUTION);
PID left_pid(-MAX_RPM, MAX_RPM, 0.8, 12.0, 0, SAMPLE_DELTA / 1000.0);
PID right_pid(-MAX_RPM, MAX_RPM, 0.8, 12.0, 0, SAMPLE_DELTA / 1000.0);

void setup() {
#ifdef DEBUG
    Serial.begin(57600);
    Serial.println("Roamer control board online");
#endif

    // I2C address 20
    slave.init(20);

    // startup sound
    // TODO: make unique
    buzzer.play("v10>>e16>>>g16");

    // allows for TEMPORARY extra power to motors
    // must ensure not constantly overdriving motor
    motors.allowTurbo(true);

    ledYellow(false);
    ledGreen(true);
    ledRed(false);
}

#ifdef DEBUG
void printDebugInfo() {
    Serial.print("BUTTONS: ");
    Serial.print(slave.buffer.buttonA);
    Serial.print(" ");
    Serial.print(slave.buffer.buttonB);
    Serial.print(" ");
    Serial.print(slave.buffer.buttonC);
    Serial.println();

    Serial.print("BATT: ");
    Serial.print(slave.buffer.batteryMillivolts, DEC);
    Serial.println("ma");

    Serial.print("Encoders: ");
    Serial.print(encoders.getCountsLeft(), DEC);
    Serial.print(" ");
    Serial.print(encoders.getCountsRight(), DEC);
    Serial.println();

    Serial.println();
}
#endif

void move() {
    // calculate required wheel RPMs for requested motion
    Kinematics::rpm req_rpm =
        kinematics.rpmForMotion(slave.buffer.twist_linear_x, slave.buffer.twist_angle_z);

    // get current RPM of each motor
    unsigned long ms = millis();
    float left_rpm = leftWheelOdom.getRPM(encoders.getCountsLeft(), ms);
    float right_rpm = rightWheelOdom.getRPM(encoders.getCountsRight(), ms);

    // calculate PID RPM values for motors based on measured vs required
    motors.setLeftSpeed(left_pid.calculate(req_rpm.left_motor, left_rpm));
    motors.setRightSpeed(right_pid.calculate(req_rpm.right_motor, right_rpm));

    // send velocity back to i2c master
    Kinematics::velocities vel = kinematics.getVelocities(left_rpm, right_rpm);
    slave.buffer.vel_linear = vel.linear;
    slave.buffer.vel_angular = vel.angular;
}

void stop() {
    // perform full stop by zeroing out command values
    // next move() command call will stop motors
    slave.buffer.twist_linear_x = 0;
    slave.buffer.twist_angle_z = 0;
}

void loop() {
    // get the latest data from master
    slave.updateBuffer();

    // READ: Button states
    slave.buffer.buttonA = buttonA.isPressed();
    slave.buffer.buttonB = buttonB.isPressed();
    slave.buffer.buttonC = buttonC.isPressed();

    // WRITE: LED values
    ledRed(slave.buffer.led_red);
    ledGreen(slave.buffer.led_green);
    ledYellow(slave.buffer.led_yellow);

    // STOP the robot if no new command received within timeout
    unsigned long ms = millis();
    static unsigned long prevCommandTime = 0;
    if (slave.buffer.new_twist_command == true) {
        // reset command timeout & mark as seen
        prevCommandTime = ms;
        slave.buffer.new_twist_command = false;
    } else if (ms - prevCommandTime >= COMMAND_TIMEOUT) {
        stop();
    }

    // DRIVE the robot
    static unsigned long lastCommandUpdate = 0;
    if (ms - lastCommandUpdate >= SAMPLE_DELTA) {
        move();
        lastCommandUpdate = ms;
    }

    // publish IMU data
    static unsigned long lastIMUPublish = 0;
    if (ms - lastIMUPublish >= IMU_PUB_RATE) {
        // TODO: publish IMU data now (is ODOM needed too?)
        lastIMUPublish = ms;
    }

    // battery low warning chime
    static unsigned long lastBatteryCheckTime = 0;
    if (ms - lastBatteryCheckTime >= BATT_CHECK_RATE) {
        // READ: Battery voltage
        slave.buffer.batteryMillivolts = readBatteryMillivolts();

        if (readBatteryMillivolts() < LOW_BATT_VALUE && !usbPowerPresent()) {
            buzzer.play("v12>>f#>>>g");
        }
        lastBatteryCheckTime = ms;
    }

    // make data available to master
    slave.finalizeWrites();

#ifdef DEBUG
    static unsigned long lastDebugTime = 0;
    if (ms - lastDebugTime >= DEBUG_RATE) {
        printDebugInfo();
        lastDebugTime = ms;
    }
#endif
}
