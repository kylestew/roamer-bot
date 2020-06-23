#include <Kinematics.h>
#include <Odometry.h>
#include <PololuRPiSlave.h>
#include <Romi32U4.h>

#define DEBUG FALSE

// TODO: double check these values
// wheel characteristics
#define WHEEL_SEPERATION 0.5     // meters
#define WHEEL_CIRCUMFERENCE 0.7  // pi * diameter

// motor characteristics
#define TICKS_PER_REVOLUTION 1440.0

// update rates
#define COMMAND_RATE 50      // 20 hz
#define COMMAND_TIMEOUT 400  // ms
#define IMU_PUB_RATE 50      // 20 hz
#define DEBUG_RATE 500       // 2 hz

struct Data {
    // 0, 1, 2
    bool led_red, led_green, led_yellow;
    // 3, 4, 5
    bool buttonA, buttonB, buttonC;
    // 6-7
    uint16_t batteryMillivolts;

    // 8-9, 10-11
    int16_t leftEncoder, rightEncoder;

    // incoming TWIST command
    bool new_twist_command;
    float twist_linear_x;
    float twist_angle_z;
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
    Serial.print(slave.buffer.leftEncoder, DEC);
    Serial.print(" ");
    Serial.print(slave.buffer.rightEncoder, DEC);
    Serial.println();

    Serial.println();
}
#endif

void move() {
    // calculate required wheel RPMs for requested motion
    Kinematics::rpm reqRPM =
        kinematics.rpmForMotion(slave.buffer.twist_linear_x, slave.buffer.twist_angle_z);

    // get current RPM of each motor
    unsigned long ms = millis();
    int leftRPM = leftWheelOdom.getRPM(encoders.getCountsLeft(), ms);
    int rightRPM = rightWheelOdom.getRPM(encoders.getCountsRight(), ms);

    // calculate PID RPM values for motors based on measured vs required
    // TODO: ...

    // TODO: push data back to PI via i2c
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

    // READ: Battery voltage
    slave.buffer.batteryMillivolts = readBatteryMillivolts();

    // WRITE: LED values
    ledRed(slave.buffer.led_red);
    ledGreen(slave.buffer.led_green);
    ledYellow(slave.buffer.led_yellow);

    // READ: encoder values
    slave.buffer.leftEncoder = encoders.getCountsLeft();
    slave.buffer.rightEncoder = encoders.getCountsRight();

    // STOP the robot if no new command received within timeout
    static unsigned long prevCommandTime = 0;
    if (slave.buffer.new_twist_command == true) {
        // reset command timeout & mark as seen
        prevCommandTime = ms;
        slave.buffer.new_twist_command = false;
    } else if (ms - prevCommandTime >= COMMAND_TIMEOUT) {
        stop();
    }

    // DRIVE the robot
    unsigned long ms = millis();
    static unsigned long lastCommandUpdate = 0;
    if (ms - lastCommandUpdate >= COMMAND_RATE) {
        move();
        lastCommandUpdate = ms;
    }

    // publish IMU data
    static unsigned long lastIMUPublish = 0;
    if (ms - lastIMUPublish >= IMU_PUB_RATE) {
        // TODO: publish IMU data now (is ODOM needed too?)
        lastIMUPublish = ms;
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
