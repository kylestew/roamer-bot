# roamer-bot

A small robotics learning platform built on the Pololu Romi chassis.

The current direction is a custom-core Studio Mission Rover: Romi chassis, custom MCU + motor-control PCB, laptop tether first, telemetry from day one, and no ROS in the core system until the low-level robot is understood.

![romi chassis](https://a.pololu-files.com/picture/0J7260.600x480.jpg?7a7be3ece5b2d0ae05a7c78b6c0770d6)

# Current Direction

The project is moving from the original Romi 32U4 + Raspberry Pi + ROS approach toward a custom-core learning build:

- Treat the old implementation as historical/reference material.
- Use the Romi chassis as the mechanical target.
- Build a custom MCU + motor-control PCB that replaces the purchasable Pololu Romi control board.
- Keep the first version tethered to a laptop.
- Capture telemetry from day one.
- Keep ROS out of the core system until the custom board, telemetry, and motor-control loop are understood.

# Current Structure

- `hardware/`: active Rev A PCB and mechanical-constraint work.
- `firmware/`: active custom MCU firmware work.
- `host/`: active laptop tether, telemetry capture, and test tooling work.
- `legacy/old-romi/`: previous Romi 32U4 + Raspberry Pi + ROS implementation, kept only as reference.
- `docs/`: datasheets, Romi reference assets, and legacy dependency references.

# Planning

- [Hardware design, release gate, and Rev B plan](hardware/README.md)
- [Studio Mission Rover roadmap](STUDIO_MISSION_ROVER_ROADMAP.md)
- [TO BUY list](TO_BUY.md)
- [Legacy Romi references](docs/legacy-romi-references.md)

# First Milestone

Teleop + encoder logging + safe stop:

- Command left and right motors from a laptop or simple host.
- Read left and right encoder counts.
- Stream basic telemetry.
- Stop motors on command timeout.
- Save a short bench or floor test log on the host.
