# 32U4 Control Board

Provides the low-level operations for the Roamer Robot:

+ Motor control (w/PID) takes ROS Twist command data
+ Encoders publish robot ODOM
+ IMU publishes robot POSE
+ Auto shutdown if new Twist command not received in a defined period
+ Buttons
+ LEDs
+ Battery voltage



### I2C Interface

Bi-directional via a struct
???



## CLI Commands

Build firmware

    $ pio run -e robot

Upload firmware via bootloader

    $ pio run -e robot -t upload

Run native desktop test

    $ pio test -e native 

Run embedded test

    $ pio test -e robot
