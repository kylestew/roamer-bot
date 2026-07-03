# roamer-bot

A small robotics learning platform built on the Pololu Romi chassis.

The current restart direction is a custom-core Studio Mission Rover: Romi chassis, custom MCU + motor-control PCB, laptop tether first, telemetry from day one, and no ROS in the core system until the low-level robot is understood.

![romi chassis](https://a.pololu-files.com/picture/0J7260.600x480.jpg?7a7be3ece5b2d0ae05a7c78b6c0770d6)

# Current Direction

The project is moving from the original Romi 32U4 + Raspberry Pi + ROS approach toward a custom-core learning build:

- Move the current implementation out of the main path and treat it as historical/reference material.
- Use the Romi chassis as the mechanical target.
- Build a custom MCU + motor-control PCB that replaces the purchasable Pololu Romi control board.
- Keep the first version tethered to a laptop.
- Capture telemetry from day one.
- Keep ROS out of the core system until the custom board, telemetry, and motor-control loop are understood.

# Current Planning

- [Electronics design spec](ELECTRONICS_DESIGN_SPEC.md)
- [Purchasing plan](PURCHASING.md)
- [Studio Mission Rover roadmap](STUDIO_MISSION_ROVER_ROADMAP.md)

# Progress:

- [x] Order parts
- [x] Build basic Romi kit with motor driver
- [x] Run motor test on 32U4 driver board 
- [ ] Write 32U4 control board firmware
- [x] Install ROS on Raspberry Pi
- [ ] Present 32U4 board as ROS node on Raspberry Pi
- [x] Basic Teleop of robot from remote computer
- [ ] Calibrate PID constants on motor driver board
- [ ] Calibrate IMU on 32U4 board?

- [ ] Create URDF for virtualization
- [ ] Full ROS/Gazebo virtualization 
- [ ] Confirm odometry in ROS
- [ ] Install LIDAR unit on robot
- [ ] Confirm LIDAR in ROS
- [ ] Setup SLAM dependencies in ROS
- [ ] Autonomous navigation!

## Libraries

- [Pololu Romi 32U4 Ardino Library](https://github.com/pololu/romi-32u4-arduino-library)
- [RomiPi Library](https://github.com/ftPeter/RomiPi)

## Examples I'm Learning From

- [RomiPi](https://github.com/ftPeter/RomiPi)
- [Linorobot](https://linorobot.org/)
- [Lidar Based Maze Rover](https://diyrobocars.com/2020/03/19/lessons-learned-making-a-lidar-based-maze-rover)
- [Phoebe](https://hackaday.io/project/161085-phoebe-turtlebot)
- [ROS TurtleBot](http://wiki.ros.org/Robots/TurtleBot)

# Building on Yourself

__ TODO: create build instructions __
