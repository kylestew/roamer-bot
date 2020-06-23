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
