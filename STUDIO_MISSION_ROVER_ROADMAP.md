# Studio Mission Rover Roadmap

Status: draft

## Thesis

The next `roamer-bot` revision should be a small, serviceable mobile robot that runs bounded indoor missions, logs telemetry, exposes its internal state, handles simple faults, and produces a post-run artifact such as a path image, run report, field note, or replay.

This is not another generic robot car. It is a learning platform for:

- Embedded motor control.
- Odometry.
- Coordinate frames.
- Sensor integration.
- State estimation.
- Autonomy modes.
- Fault handling.
- Telemetry.
- Replay.
- Human-facing debugging.
- Artifact generation.

## Architecture Stance

Build the core system yourself first. Do not start with ROS.

For v0 and probably v1, the rover should use a custom, understandable stack:

- MCU firmware for motor control, encoder reading, watchdogs, and hardware fault flags.
- Custom rover electronics, including the motor-driver board or motor-driver section.
- A small serial protocol between MCU and host.
- A custom mission runner on laptop first.
- Explicit odometry code.
- Explicit coordinate-frame structs and transforms.
- A custom telemetry log format.
- A simple ground console.
- A custom replay/artifact generator.

Add ROS later only when the custom core is already working and the reason is concrete:

- Bridge logs into standard visualization tools.
- Compare custom frame handling with `tf2`.
- Record or replay with bags.
- Experiment with Nav2.
- Integrate a sensor whose ROS driver saves major effort.

Rule:

> Build the robot's nervous system yourself first. Add ROS only when you can explain exactly what it is replacing.

## First Milestone

Teleop + encoder logging + safe stop.

That means:

- Custom board can command left/right motors.
- Encoders are read reliably.
- Host can send commands over USB/serial.
- Board streams basic telemetry.
- Command timeout stops the motors.
- A short run log is saved on the host.

## What This Teaches

Algorithm experience:

- Differential-drive kinematics.
- Encoder odometry.
- PID or velocity control.
- Sensor filtering.
- Coordinate frames.
- Obstacle detection later.
- Local navigation behaviors later.
- State machines.
- Fault detection and recovery.

Systems integration experience:

- MCU firmware and host software cooperating.
- Motors, encoders, batteries, sensors, and compute working as one system.
- Timing and message rates being managed.
- Telemetry being structured.
- Hardware failures becoming diagnosable.
- Run data becoming a tool for improvement.

## What To Defer

- ROS.
- SLAM.
- Nav2.
- Cameras.
- Onboard computer.
- IMU.
- Custom mechanical chassis.
- Full Romi 32U4 feature clone.

## Near-Term Sequence

1. Order Romi chassis/encoder parts.
2. Inspect physical chassis and encoder geometry.
3. Extract mounting holes, encoder mating geometry, battery contact geometry, and clearances.
4. Decide whether to build Board 0 or jump to Board 1.
5. Choose MCU and motor-driver IC.
6. Design Board 1 schematic.
7. Design Board 1 PCB around Romi mounting/encoder constraints.
8. Bring up motor drive, encoders, telemetry, and safe stop before adding higher-level behavior.
