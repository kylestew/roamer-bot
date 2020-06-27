import time
from romi.romi32U4_driver import Romi32U4Driver

romi = Romi32U4Driver()

romi.leds(1, 0, 0)
time.sleep(1)
romi.leds(0, 1, 0)
time.sleep(1)
romi.leds(0, 0, 1)
time.sleep(1)

