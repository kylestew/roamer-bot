#include "Kinematics.h"

void Kinematics::setTwistTarget(float linear, float angle) {
    // calculate velocities at wheel center
    // rotate CCW by applying more velocity to right wheel (per angle value)
    // left wheel needs to do negative angle component to rotate properly
    float right_vel = (angle * _wheel_seperation) / 2.0 + linear;
    float left_vel = (linear * 2.0) - right_vel;

    // set target velocities
    _wheel_velocities.left_wheel = left_vel;
    _wheel_velocities.right_wheel = right_vel;
}

Kinematics::velocities Kinematics::getVelocities() { return _wheel_velocities; }

/*
                long encoder_ticks = read();
                //this function calculates the motor's RPM based on encoder ticks and delta time
                unsigned long current_time = millis();
                unsigned long dt = current_time - prev_update_time_;

                //convert the time from milliseconds to minutes
                double dtm = (double)dt / 60000;
                double delta_ticks = encoder_ticks - prev_encoder_ticks_;

                //calculate wheel's speed (RPM)

                prev_update_time_ = current_time;
                prev_encoder_ticks_ = encoder_ticks;

                return (delta_ticks / counts_per_rev_) / dtm;
   */
