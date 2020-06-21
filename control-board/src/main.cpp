#include <PololuRPiSlave.h>
#include <Romi32U4.h>

#include "odometry.h"

#define DEBUG FALSE

struct Data {
    // 0, 1, 2
    bool led_red, led_green, led_yellow;
    // 3, 4, 5
    bool buttonA, buttonB, buttonC;
    // 6-7
    uint16_t batteryMillivolts;

    // 8-9, 10-11
    int16_t leftEncoder, rightEncoder;
};

PololuRPiSlave<struct Data, 10> slave;

PololuBuzzer buzzer;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
Romi32U4ButtonC buttonC;
Romi32U4Encoders encoders;

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

void loop() {
    // get the latest data from master
    slave.updateBuffer();

    // read values into buffer
    slave.buffer.buttonA = buttonA.isPressed();
    slave.buffer.buttonB = buttonB.isPressed();
    slave.buffer.buttonC = buttonC.isPressed();

    slave.buffer.batteryMillivolts = readBatteryMillivolts();

    // READ: LED values
    ledRed(slave.buffer.led_red);
    ledGreen(slave.buffer.led_green);
    ledYellow(slave.buffer.led_yellow);

    // READ: encoder values
    slave.buffer.leftEncoder = encoders.getCountsLeft();
    slave.buffer.rightEncoder = encoders.getCountsRight();

    // make data available to master
    // slave.finalizeWrites();

#ifdef DEBUG
    static unsigned long lastDebugTime = 0;

    if (millis() - lastDebugTime >= 500) {
        Serial.print("BUTTONS: ");
        Serial.print(slave.buffer.buttonA);
        Serial.print(" ");
        Serial.print(slave.buffer.buttonA);
        Serial.print(" ");
        Serial.print(slave.buffer.buttonA);
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
}
