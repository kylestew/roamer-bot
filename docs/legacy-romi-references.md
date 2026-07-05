# Legacy Romi References

The old Romi 32U4 + Raspberry Pi + ROS implementation has been moved to `legacy/old-romi/`.

These projects used to be Git submodules. They are no longer active dependencies of this repo, but the old URLs and pinned commits are preserved here for historical reference.

| Project | URL | Old pinned commit |
| --- | --- | --- |
| Pololu Romi 32U4 Arduino Library | https://github.com/pololu/romi-32u4-arduino-library.git | `86ed35f2ad56947020091cefc4921d37ab0378cd` |
| Linorobot PID | https://github.com/linorobot/lino_pid.git | `e248c5e8e5f2e4961d8f2d188421c09610b43e85` |
| Linorobot messages | https://github.com/linorobot/lino_msgs.git | `e6a37f7deb8351d7022eaf37b9af9277f3a965cf` |

The active custom-core project should not depend on these packages. Use the old code only for reference patterns such as command timeout, PID structure, odometry math, kinematics separation, I2C message shape, and battery handling.
