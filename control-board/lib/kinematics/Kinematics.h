#pragma once

class Kinematics {
public:
    struct velocities {
        float left_wheel;
        float right_wheel;
    };

    Kinematics(float wheel_seperation) : _wheel_seperation(wheel_seperation) {}

    // x: m/s - z: rad/s
    void setTwistTarget(float linear, float angle);
    velocities getVelocities();

private:
    // in cm
    float _wheel_seperation;
    float _wheel_circumference;

    velocities _wheel_velocities{0, 0};
};

