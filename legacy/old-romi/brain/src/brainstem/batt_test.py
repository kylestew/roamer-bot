from scripts.romi32U4_driver import Romi32U4Driver

romi = Romi32U4Driver()

val = romi.read_battery_millivolts()
print("batt reads {:} mv".format(val))

