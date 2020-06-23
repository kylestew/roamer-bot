#pragma once

class Odometry {
public:
    Odometry(float ticks_per_revolution)
        : _ticks_per_revolution(ticks_per_revolution),
          _last_update_time(0),
          _last_encoder_tick(0) {}

    int getRPM(uint16_t encoder_ticks, unsigned long current_millis);

private:
    float _ticks_per_revolution;
    unsigned long _last_update_time;  // ms
    uint16_t _last_encoder_tick;
};
