#pragma once

class Odometry {
public:
    Odometry(float ticks_per_revolution)
        : _ticks_per_revolution(ticks_per_revolution),
          _last_update_time(0),
          _last_encoder_ticks(0) {}

    // time in ms
    float getRPM(int encoder_ticks, unsigned long current_time);

private:
    float _ticks_per_revolution;
    unsigned long _last_update_time;  // ms
    int _last_encoder_ticks;
};
