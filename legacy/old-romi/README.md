# Old Romi Implementation

This directory contains the previous Romi 32U4 + Raspberry Pi + ROS implementation.

It is kept as historical/reference material only. The active project is the custom-core Studio Mission Rover work in `hardware/`, `firmware/`, and `host/`.

Useful reference areas:

- `control-board/`: 32U4 firmware, motor PID, encoder RPM, kinematics, odometry, command timeout, and battery reporting.
- `brain/`: old ROS workspace, I2C host driver, `cmd_vel` bridge, odometry publishing, and battery publishing.

The old Git submodules are intentionally removed. Their URLs and pinned commits are documented in [Legacy Romi references](../../docs/legacy-romi-references.md).
