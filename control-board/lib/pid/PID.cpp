#include "PID.h"

#include "Arduino.h"

PID::PID(float min, float max, float Kp, float Ki, float Kd, float dt)
    : _min(min), _max(max), _kp(Kp) {
    _ki = Ki * dt;
    _kd = Kd / dt;
}

float PID::calculate(float goal, float measured) {
    float error = goal - measured;

    //     limit to maximum to avoid accumulation of errors via saturation

    // accumulate errors as integral
    _integral += error;

    float pid = (_kp * error) + (_ki * _integral);

    return constrain(pid, _min, _max);
}

