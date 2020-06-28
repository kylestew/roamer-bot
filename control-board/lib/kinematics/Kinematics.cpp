#include "Kinematics.h"

Kinematics::rpm Kinematics::rpmForMotion(float linear, float angular) {
    // convert angular velocity to tangential velocity
    float tangential_vel = angular * (_seperation / 2.0);

    // convert motion values to RPM components
    float linear_rpm = (linear / _circumference) * RPS_TO_RPM;
    float tangential_rpm = (tangential_vel / _circumference) * RPS_TO_RPM;

    // assign to wheels, rotation is CCW
    Kinematics::rpm rpm;
    rpm.left_motor = linear_rpm - tangential_rpm;
    rpm.right_motor = linear_rpm + tangential_rpm;
    return rpm;
}

Kinematics::velocities Kinematics::getVelocities(int left_motor_rpm, int right_motor_rpm) {
    Kinematics::velocities vel;

    // RPMs -> velocities

    // linear velocity is the average of the wheel velocities
    float avg_rps_x = (left_motor_rpm + right_motor_rpm) / 60.0;
    vel.linear = avg_rps_x * _circumference;

    // angular velocity relates to the RPM difference between between wheels
    float avg_rps_a = (right_motor_rpm - left_motor_rpm) / 60.0;
    vel.angular = (avg_rps_a * _circumference) / (_seperation / 2);

    return vel;
}

