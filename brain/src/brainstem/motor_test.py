import time
from scripts.romi32U4_driver import Romi32U4Driver

romi = Romi32U4Driver()

forward_speed_m_s = 0
rot_speed_rad_s = 0

def command_and_monitor(duration_s):
    update_freq_hz = 4.0

    romi.leds(1, 0, 0)
    for i in range(0, int(duration_s * update_freq_hz)):
        romi.twist(forward_speed_m_s, rot_speed_rad_s)

        print("{:}\tlinear m/s {:}\tangular rad/s {:}".format(
            i / update_freq_hz,
            romi.read_linear_velocity(), 
            romi.read_angular_velocity()))
        time.sleep(1.0 / update_freq_hz)
    romi.leds(0, 0, 0)
    print("================================")

def stop(pause):
    romi.twist(0, 0)
    time.sleep(pause)

# TEST FORWARD
print("forward 20cm")
# 0.1 m/s for 2 seconds
forward_speed_m_s = 0.1
rot_speed_rad_s = 0.0
command_and_monitor(2)

stop(2)

# TEST ROTATE
print("rotate 180 deg")
# pi rad/s for 1 second (1/4 turn)
forward_speed_m_s = 0.0
rot_speed_rad_s = 3.14159 / 2.0
command_and_monitor(2)

stop(2)

print("stopping")
forward_speed_m_s = 0
rot_speed_rad_s = 0
command_and_monitor(2)

print("test complete")

