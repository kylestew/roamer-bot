# 32U4 Control Board

Provides the low-level operations for the Roamer Robot:

+ Motor control (w/PID)
+ IMU (TODO: what can be read here?)
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
