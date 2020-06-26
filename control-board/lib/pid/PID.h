#pragma once

class PID {
public:
    PID(float min, float max, float kp, float ki, float kd);
    float compute(float goal, float measured);

private:
    float _min;
    float _max;
    float _kp;
    float _ki;
    float _kd;
    float _integral;
};
