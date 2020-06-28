#pragma once

#define RPS_TO_RPM 60.0

class Kinematics {
public:
    struct rpm {
        float left_motor;
        float right_motor;
    };

    struct velocities {
        float linear;
        float angular;
    };

    Kinematics(float circumference, float seperation)
        : _circumference(circumference), _seperation(seperation) {}

    // linear x: m/s - angular z: rad/s
    rpm rpmForMotion(float linear, float angular);

    velocities getVelocities(int left_motor_rpm, int right_motor_rpm);

private:
    // in meters
    float _circumference;
    float _seperation;
};

