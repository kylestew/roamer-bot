#pragma once

class PID {
public:
    // min / max - output saturation point
    // Kp - proportional gain
    // Ki - integral gain
    // Kd - deriviative gain
    // dt - time between samples (in seconds)
    PID(float min, float max, float Kp, float Ki, float Kd, float dt);
    float calculate(float goal, float measured);

private:
    float _min;
    float _max;
    float _kp;
    float _ki;
    float _kd;

    float _integral;
};
