#include "Odometry.h"

float Odometry::getRPM(unsigned int encoder_ticks, unsigned long current_time) {
    // convert delta time from millis to minutes (for RPM)
    double dtm = (current_time - _last_update_time) / 60000.0;
    double delta_ticks = encoder_ticks - _last_encoder_ticks;

    _last_update_time = current_time;
    _last_encoder_ticks = encoder_ticks;

    // RPM = ticks / ticks per revolution / time
    return (delta_ticks / _ticks_per_revolution) / dtm;
}

