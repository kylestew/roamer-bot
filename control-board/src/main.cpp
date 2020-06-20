#include <Romi32U4.h>

#include "odometry.h"

#define DEBUG TRUE

PololuBuzzer buzzer;

Odometry odom;

void setup() {
#ifdef DEBUG
    Serial.begin(57600);
    Serial.println("Roamer control board online");
#endif

    // startup sound
    // TODO: make unique
    buzzer.play("v10>>e16>>>g16");

    ledYellow(false);
    ledGreen(true);
    ledRed(false);
}

void loop() {
    // TODO: do no poll here

    uint16_t battV = readBatteryMillivolts();
    Serial.print("batt: ");
    Serial.println(battV, DEC);

    // odom.calculate();

    delay(1000);
}

