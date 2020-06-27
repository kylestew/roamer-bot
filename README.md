# roamer-bot

A small SLAM capable robot built on the Pololu Romi chassis

![romi chassis](https://a.pololu-files.com/picture/0J7260.600x480.jpg?7a7be3ece5b2d0ae05a7c78b6c0770d6)

# Progress:

- [x] Order parts
- [x] Build basic Romi kit with motor driver
- [x] Run motor test on 32U4 driver board 
- [ ] Write 32U4 control board firmware
- [x] Install ROS on Raspberry Pi
- [ ] Present 32U4 board as ROS node(s) on Raspberry Pi
- [ ] Basic Teleop of robot from remote computer
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

