#include "PID.h"

#include "Arduino.h"

PID::PID(float min, float max, float kp, float ki, float kd)
    : _min(min), _max(max), _kp(kp), _ki(ki), _kd(kd) {}

float PID::compute(float goal, float measured) {
    float error = goal - measured;

    // accumulate errors as integral
    _integral += error;

    float pid = (_kp * error) + (_ki * _integral);

    return constrain(pid, _min, _max);
}

