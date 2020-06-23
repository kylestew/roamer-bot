#pragma once

#define RPM_TO_RPS 1 / 60

class Kinematics {
public:
    struct rpm {
        int left_motor;
        int right_motor;
    };

    // struct velocities {
    //     float left_wheel;
    //     float right_wheel;
    // };

    Kinematics(float circumference, float seperation)
        : _circumference(circumference), _seperation(seperation) {}

    // linear x: m/s - angle z: rad/s
    rpm rpmForMotion(float linear, float angle);

private:
    // in meters
    float _circumference;
    float _seperation;
};

